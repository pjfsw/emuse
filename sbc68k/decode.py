import configparser
import sys
import re

cfg = configparser.ConfigParser(allow_no_value=True)
cfg.read("decode.ini")

pin_by_name={}
name_by_pin={}
address_fields = {}
macros = {}
macro_ref = re.compile(r"\{([A-Z0-9_]+)\}")

def expand_macros(expr: str, macros: dict[str, str]) -> str:
    def replace(match):
        name = match.group(1)
        if name not in macros:
            sys.exit(f"Unknown macro {{{name}}} in expression: {expr}")
        return macros[name]

    return macro_ref.sub(replace, expr)

def add_pin(name: str, pin: int):
    #print(f"Adding {name}={pin}")
    if name in pin_by_name:
        old_pin = pin_by_name[name]
        sys.exit(f"Error: signal '{name}' is already assigned to pin {old_pin}")

    if pin in name_by_pin:
        old_name = name_by_pin[pin]
        sys.exit(f"Error: pin {pin} is already assigned to signal '{old_name}', cannot also assign '{name}'")

    pin_by_name[name] = pin
    name_by_pin[pin] = name

def expand_addr_match(field_name: str, value_text: str) -> str:
    pins = address_fields[field_name.upper()]
    value = int(value_text, 0)   # accepts 12, 0xc, 0b1100

    if value >= (1 << len(pins)):
        raise ValueError(f"{field_name} value {value_text} does not fit in {len(pins)} bits")

    terms = []

    for i, pin in enumerate(reversed(pins)):
        bit = (value >> i) & 1
        terms.append(pin if bit else f"/{pin}")

    return " * ".join(terms)


def print_pins(start:int, end: int):
    pins=[]
    for pin in range(start,end+1):
        pins.append(name_by_pin.get(pin, "NC"))
    print(" ".join(pins))

def add_macro(macro,value):
    if macro in macros:
        sys.exit(f"Error: macro '{macro}' is already defined as {value}")                
    macros[macro] = value

def print_equation(output: str, expr: str):
    terms = [t.strip() for t in expr.split("+")]

    prefix = f"{output} "

    print(prefix + "= " + terms[0])

    indent = " " * len(prefix)

    for term in terms[1:]:
        print(indent + "+ " + term)

def main():
    add_pin("CLK",1)
    add_pin("GND",12)
    add_pin("VCC",24)

    for name, pin in cfg["pins"].items():
        add_pin(name.upper(), int(pin))

    for field_name, pin_list in cfg["addresspins"].items():
        pins = [p.strip().upper() for p in pin_list.split(",")]
        address_fields[field_name.upper()] = pins

    for field_name, address_string in cfg["addresses"].items():
        address_name = field_name.upper()
        address_mask, address_val = address_string.strip().upper().split(":",1)
        address_val = address_val.replace("$", "0X")
        #print(f"name {address_name}, {address_mask}, {address_val}")
        add_macro(address_name,expand_addr_match(address_mask, address_val)) 

    for field_name, value in cfg["macros"].items():
        macro = field_name.upper()
        add_macro(macro, expand_macros(value,macros))

    print_pins(1,12)
    print_pins(13,24)
    print("")
    for field_name, value in cfg["decode"].items():
        output=field_name.upper()
        print_equation(output,expand_macros(value,macros))

main()