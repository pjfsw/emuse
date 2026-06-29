;____________________________________________________________
;
; Read the 32-bit little endian in address in A0 and store in D0
;____________________________________________________________

ReadLe32:
    move.b 3(a0),d0
    lsl.l  #8,d0
    move.b 2(a0),d0
    lsl.l  #8,d0
    move.b 1(a0),d0
    lsl.l  #8,d0
    move.b 0(a0),d0
    rts

ReadLe16:    
    move.b 1(a0),d0
    lsl.w  #8,d0
    move.b 0(a0),d0
    and.l  #$ffff,d0
    rts
