import configparser
import sys

cfg = configparser.ConfigParser(allow_no_value=True)
cfg.read("decode.ini")

pin_by_name={}
name_by_pin={}
address_fields = {}

def add_pin(name: str, pin: int):
    print(f"Adding {name}={pin}")
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
        terms.append(pin if bit else f"!{pin}")

    return " & ".join(terms)


def print_pins(start:int, end: int):
    pins=[]
    for pin in range(start,end+1):
        pins.append(name_by_pin.get(pin, "NC"))
    print(" ".join(pins))

def main():
    add_pin("CLK",1)
    add_pin("GND",12)
    add_pin("VCC",24)

    for name, pin in cfg["pins"].items():
        add_pin(name.upper(), int(pin))

    for field_name, pin_list in cfg["addresspins"].items():
        pins = [p.strip().upper() for p in pin_list.split(",")]
        address_fields[field_name.upper()] = pins

    print(expand_addr_match("ADDR4", "0xc"))

main()