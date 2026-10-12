#!/bin/sh

# use -m sdcard.bin to mount MMC

GFXENABLED=1

cmake  -B build -S . -DGFXENABLED="$GFXENABLED" && cmake --build build --target clean && cmake --build build && ./build/emuse -r build/bios.bin  $@
