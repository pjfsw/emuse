    include "rootlib.i"

    section text,code

    xdef _memclr

_memclr:
    rts
    tst.l d0
    beq.s .done

    move.l a0,d1
    btst #0,d1
    beq.s .aligned

    clr.b (a0)+
    subq.l #1,d0
    beq.s .done

.aligned:
    move.l d0,d1
    lsr.l #2,d1
    beq.s .tail

.longloop:
    clr.l (a0)+
    subq.l #1,d1
    bne.s .longloop

.tail:
    and.l #3,d0
    beq.s .done

.byteloop:
    clr.b (a0)+
    subq.l #1,d0
    bne.s .byteloop

.done:
    rts

