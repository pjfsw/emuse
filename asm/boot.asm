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
    lea (welcomeMsg).l,a1
    bsr writeMessage
    bsr waitKey
    lea (anotherMsg).l,a1
    bsr writeMessage    
    ;bsr MMCStartTransfer
    ;bsr MMCSendByte
    ;bsr MMCReadByte
    ;bsr MMCEndTransfer
loop:
    bra loop
welcomeMsg:
    dc.b "Hello world from 68000! Press space!",13,10,0
    even
anotherMsg:
    dc.b "You pressed a key, splendid!",13,10,0
    even

writeMessage:
writeMessage_nextChar:
    move.b (a1)+,d0
    beq.s writeMessage_done
    bsr UARTPutChar
    bra.s writeMessage_nextChar
writeMessage_done:
    rts    

waitKey:
    bsr UARTReadCharBlocking
    rts        

    include mmc.asm
    include uart.asm
