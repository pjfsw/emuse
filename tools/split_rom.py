import sys

TOTAL_SIZE = 64 * 1024      
PAD_BYTE = 0xFF            

with open(sys.argv[1], "rb") as f:
    data = f.read()

data += bytes([PAD_BYTE]) * (TOTAL_SIZE - len(data))

with open(sys.argv[2], "wb") as f:
    f.write(data[0::2])   # HI ROM

with open(sys.argv[3], "wb") as f:
    f.write(data[1::2])   # LO ROM

