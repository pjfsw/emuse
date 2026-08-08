#!/bin/sh

# use -m sdcard.bin to mount MMC

cmake -B build -S . && cmake --build build --target clean && cmake --build build && ./build/emuse -r build/bios.bin  $@
