RAM_SIZE equ $100000

; boot.asm - Minimal 68k Header
    org $f00000

    dc.l RAM_SIZE     ; Initial Stack Pointer (Top of RAM)
    dc.l Start        ; Initial Program Counter (Where code begins)

    org $f00400        ; Move past the vector table
Start:   
    lea $11000,a0
    move.l #$12345678,d0
    move.l d0,(a0)
    move.l $11000,d1
    bra MMCStartTransfer
    
    include mmc.asm
