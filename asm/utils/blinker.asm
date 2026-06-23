    incdir ..
    include hardware.i

    macro Delay
    move.l #$24000,d0
loop\@:
    subq.l #1,d0
    bne.s loop\@
    endm

START equ $10000
    org START

    lea OREG+SPI_CS,a0
loop:
    move.b #3,(a0)
    Delay
    move.b #0,(a0)
    Delay
    bra loop

