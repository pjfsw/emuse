    include "hardware.i"

; boot.asm - Minimal 68k Header
    org $f00000

    dc.l RAM_SIZE     ; Initial Stack Pointer (Top of RAM)
    dc.l Start        ; Initial Program Counter (Where code begins)

    org $f00400        ; Move past the vector table
Start:   
    lea OREG,a5
    
    move.b #OVR_OFF,OVR(a5) ; Enable RAM     
blink:
    move.b #3,SPI_CS(a5)    ; Turn on LED
    bsr delay
    move.b #0,SPI_CS(a5)    ; Turn off LED
    bsr delay
    bra blink
    bsr MMCStartTransfer
    ;bsr MMCSendByte
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
