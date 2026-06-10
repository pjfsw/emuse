import sys

with open(sys.argv[1], "rb") as f:
    data = f.read()

with open(sys.argv[2], "wb") as f:
    f.write(data[0::2])   # HI ROM

with open(sys.argv[3], "wb") as f:
    f.write(data[1::2])   # LO ROM

