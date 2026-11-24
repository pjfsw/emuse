#!/bin/sh
set -e 

"$(dirname "$0")"/asm.sh jofmodorese/legacy/shell/shell.asm 
#cp build/shell.bin build/system.bin
