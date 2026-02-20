; boot.asm - Minimal 68k Header
    org $00000000

    dc.l $00040000    ; Initial Stack Pointer (Top of RAM)
    dc.l Start        ; Initial Program Counter (Where code begins)

    org $00000400        ; Move past the vector table
Start:
    move.l  #$1234,d0 ; Put something in a register to test
    bra     Start        ; Infinite loop
