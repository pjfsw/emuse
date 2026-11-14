#!/bin/sh

# use -m sdcard.bin to mount MMC

GFXENABLED=0

cmake  -B build -S . -DGFXENABLED="$GFXENABLED" && cmake --build build --target clean && cmake --build build && ./build/emuse -r build/bios.bin  $@
