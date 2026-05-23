RAM_SIZE equ $100000

; boot.asm - Minimal 68k Header
    org $f00000

    dc.l RAM_SIZE     ; Initial Stack Pointer (Top of RAM)
    dc.l Start        ; Initial Program Counter (Where code begins)

    org $f00400        ; Move past the vector table
Start:   
    move.l #$4000000,d0
x:    
    add.l d0,d1
    bra x
    bsr MMCStartTransfer
    bsr MMCSendByte
loop:
    bra loop
    
    include mmc.asm
