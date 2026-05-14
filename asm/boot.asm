; boot.asm - Minimal 68k Header
    org $00000000

    dc.l $00040000    ; Initial Stack Pointer (Top of RAM)
    dc.l Start        ; Initial Program Counter (Where code begins)

    org $00000400        ; Move past the vector table
Start:
    move.b #$aa,d2
    move.l #$12345678,d2 
    move.b d2,d4
    move.w #$9999,d2
    move.w d2,d3
    bra     Start        ; Infinite loop
