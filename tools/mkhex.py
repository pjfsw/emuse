#!/usr/bin/env python3

import sys

if len(sys.argv) != 3:
    print(f"Usage: {sys.argv[0]} <input.bin> <output.hex>")
    sys.exit(1)

with open(sys.argv[1], "rb") as fin:
    data = fin.read()

with open(sys.argv[2], "w") as fout:
    fout.write("U\n")
    for i, b in enumerate(data):
        fout.write(f"{b:02X}")

        # Optional formatting: 32 bytes per line
        if (i + 1) % 32 == 0:
            fout.write("\n")

    fout.write("\n.\n")

