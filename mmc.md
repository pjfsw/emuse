Grabbing an MMC image
=====================
1. Unmount MMC
2. Identify MMC: lsblk -f
3. Grab RAW data to a file (here the first 32 MB):
   sudo dd if=/dev/sdb of=./sdcard.bin bs=32M count=1 status=none

