avr-gcc -mmcu=attiny2313 -DF_CPU=1000000UL -Os reset.c -o reset.elf
avr-objcopy -O ihex reset.elf reset.hex
avrdude -c arduino -P /dev/ttyACM0 -p attiny2313 -b 19200  -U flash:w:reset.hex

