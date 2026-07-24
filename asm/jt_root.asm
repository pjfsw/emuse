;____________________________________________________________
;
; Jump table for the root library
;____________________________________________________________

JT_LibAdd:
    jmp 0
JT_LibClose:
    jmp 0
JT_LibOpen:
    jmp 0
JT_MemAvail:
    jmp MemAvail
JT_MemFree:
    jmp MemFree
JT_MemAlloc:
    jmp MemAlloc
JT_ConGetChar: ; -52
    jmp ConGetChar
JT_ConReserved0 ; -46
    blk.b 6,0
JT_ConPutHex8:  ; -40
    jmp ConPutHex8
JT_ConPutHex16: ; -34
    jmp ConPutHex16
JT_ConPutHex32: ; -28
    jmp ConPutHex32
JT_ConPuts: ; -22
    jmp ConPuts
JT_ConPutc: ; -16
    jmp ConPutc
JT_ConClr: ; -10
    jmp ConClr
    dc.l 1             ; Version

