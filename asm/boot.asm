    include "hardware.inc"

; boot.asm - Minimal 68k Header
    org $f00000

    dc.l RAM_SIZE     ; Initial Stack Pointer (Top of RAM)
    dc.l Start        ; Initial Program Counter (Where code begins)

    org $f00400        ; Move past the vector table
Start:   
    move.b #OVR_OFF,OVR
    bsr MMCStartTransfer
    ;bsr MMCSendByte
    bsr MMCReadByte
    bsr MMCEndTransfer
loop:
    bra loop
    
    include mmc.asm
