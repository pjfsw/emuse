#!/bin/sh

cmake -B build -S . && cmake --build build --target clean && cmake --build build && ./build/emuse -r build/bios.bin -m sdcard.bin $@
#cmake -B build -S . && cmake --build build --target clean && cmake --build build && ./build/emuse -r build/bios.bin  $@
