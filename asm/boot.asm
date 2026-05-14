; boot.asm - Minimal 68k Header
    org $00000000

    dc.l $00040000    ; Initial Stack Pointer (Top of RAM)
    dc.l Start        ; Initial Program Counter (Where code begins)

    org $00000400        ; Move past the vector table
Start:
    move.b #$aa,d5
    move.l #$12345678,d2 
    move.b d2,d4
    move.w #$9999,d2
    move.w d2,d3
    move.w d3,d5
    move.w #0,d2
    bra.s Start        ; Infinite loop
    move.w #7,d7
