    include "rootlib.i"
    
    section text,code

    xdef _puts

_puts:
    move.l a6,-(sp)
    move.l ROOTLIB_BASE,a6
    jsr CONPUTS(a6)
    move.l (sp)+,a6
    rts
