SBC68K hardware specification
=============================

Address map 
-----------

- `$000000-$0FFFFF` 1 MB RAM
- `$100000-$9FFFFF` Reserved
- `$A00000-$AFFFFF` Timer IRQ acknowledge
- `$B00000-$B0000E` 16C550 (even addresses)
- `$C00000-$C0003F` Graphics port
- `$D00000-$D0000E` OREG
- `$E00000-$E00000` IREG
- `$F00000-$F0FFFF` 64 KB ROM

Address decode
--------------
Inputs: A23,A22,A21,A20, /LDS, /UDS, /AS, RWB, /OVR


RAM Hi CS = 
RAM Lo CS = 
ROM CS = 

Graphics Port CS = 