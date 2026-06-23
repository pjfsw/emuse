    incdir ..
    include hardware.i

    macro Delay
    move.l #$34000,d0
loop\@:
    subq.l #1,d0
    bne.s loop\@
    endm

START equ $10000
    org START
    move.l $4,a6
    lea OREG+SPI_CS,a5
    moveq #0,d2
loop:
    addq.w #1,d2
    move.w d2,d0
    jsr -34(a6)  ; conputhex16
    lea Msg(pc),a1
    jsr -22(a6)  ; conputs
    move.b #3,(a5)
    Delay
    move.b #0,(a5)
    Delay
    bra loop
Msg:
    dc.b "!",13,10,0
    even