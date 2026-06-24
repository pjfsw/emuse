#!/bin/bash

set -e

INPUT="$1"

if [ -z "$INPUT" ]; then
    echo "Usage: $0 <file.asm>"
    exit 1
fi

BASENAME=$(basename "$INPUT" .asm)

mkdir -p build

BINFILE="build/${BASENAME}.bin"
HEXFILE="build/${BASENAME}.hex"
SYMFILE="build/${BASENAME}.sym"
LSTFILE="build/${BASENAME}.lst"

vasmm68k_mot \
    -ignore-mult-inc \
    -Fbin \
    -L "$LSTFILE" \
    -Lnf \
    -no-opt \
    -spaces \
    "$INPUT" \
    -o "$BINFILE"

python mkhex.py "$BINFILE" "$HEXFILE"

echo "Generated:"
echo "  $BINFILE"
echo "  $HEXFILE"
