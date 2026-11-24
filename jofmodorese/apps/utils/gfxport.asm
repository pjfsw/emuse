    incdir ..
    include hardware.i

    macro Delay
    move.l #$10,d0
loop\@:
    subq.l #1,d0
    bne.s loop\@
    endm

    org $1000

    lea GFXPORT_BASE,a0
    moveq #0,d1
    moveq #1,d2    
loop:
    move.b d1,1(a0)
    move.b d2,1(a0)
    bra.s loop
    
blink:
