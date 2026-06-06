    include "hardware.i"

; boot.asm - Minimal 68k Header
    org $f00000

    dc.l RAM_SIZE     ; Initial Stack Pointer (Top of RAM)
    dc.l Start        ; Initial Program Counter (Where code begins)

    org $f00400        ; Move past the vector table
Start:   
    move.b #OVR_OFF,OVR_REG
    bsr UARTInit
    bsr MMCStartTransfer
    bsr MMCSendByte
    bsr MMCReadByte
    bsr MMCEndTransfer
loop:
    bra loop
    
delay:
    move.l #$fff80000,d7
loop\@:
    addq.l #1,d7
    bne loop\@
    rts

    include mmc.asm
    include uart.asm
