    include "hardware.i"

; boot.asm - Minimal 68k Header
    org $f00000

    dc.l RAM_SIZE     ; Initial Stack Pointer (Top of RAM)
    dc.l Start        ; Initial Program Counter (Where code begins)

    org $f00400        ; Move past the vector table
Start:   
    move.b #OVR_OFF,OVR_REG
    ;lea $40000,a0
    ;move.b #1,(a0)
    ;btst.b #0,(a0)
    ;btst.b #1,(a0)
    bsr UARTInit
    bsr writeWelcomeMessage
    bsr MMCStartTransfer
    bsr MMCSendByte
    bsr MMCReadByte
    bsr MMCEndTransfer
loop:
    bra loop

writeWelcomeMessage:
    lea (welcomeMsg\@).l,a1
nextChar\@:
    move.b (a1)+,d0
    beq.s done\@
    bsr UARTPutChar
done\@:
    rts    
welcomeMsg\@:
    dc.b "Hello world!", 13,10,0
    even

    include mmc.asm
    include uart.asm
