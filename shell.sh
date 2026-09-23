#!/bin/sh
set -e 

./asm.sh asm/shell/shell.asm 
cp build/shell.bin build/system.bin
