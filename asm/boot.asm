; boot.asm - Minimal 68k Header
    org $00000000

    dc.l $00040000    ; Initial Stack Pointer (Top of RAM)
    dc.l Start        ; Initial Program Counter (Where code begins)

    org $00000400        ; Move past the vector table
Start:
    move.w #$01,d0
    move.l #$1000002,d0
    move.w #$11,d0
    move.l #$1000012,d0
    move.w #$21,d0
    move.l #$1000022,d0
    move.w #$31,d0    
    move.l #$1000032,d0
    move.w #$01,d0
    move.l #$1000002,d0
    move.w #$11,d0
    move.l #$1000012,d0
    bra Foo
    move.w #$21,d0
    move.l #$1000022,d0
    move.w #$31,d0    
    move.l #$1000032,d0
    move.w #$01,d0
    move.l #$1000002,d0
    move.w #$11,d0
    move.l #$1000012,d0
    move.w #$21,d0
    move.l #$1000022,d0
    move.w #$31,d0    
    move.l #$1000032,d0
Foo:    
    move.w #$01,d0
    move.l #$1000002,d0
    move.w #$11,d0
    move.l #$1000012,d0
    move.w #$21,d0
    move.l #$1000022,d0
    move.w #$31,d0    
    move.l #$1000032,d0


Middle:    
    move.w #$41,d0
    move.l #$1000042,d0
    move.w #$51,d0
    move.l #$1000052,d0
    move.w #$61,d0
    move.l #$1000062,d0
    move.w #$71,d0
    move.l #$1000072,d0
    move.w #$81,d0
    move.l #$1000082,d0
    bra.s Middle

    move.w #$9234,a0
    move.l a0,a1
    move.w #$1234,a0
    move.w a0,d0
    move.b #$aa,d5
    move.l #$12345678,d2 
    move.b d2,d4
    move.w #$9999,d2
    move.w d2,d3
    move.w d3,d5
    move.w #0,d2
    move.w #7,d7
    move.l #17,a0
    btst #6,$bfe001
