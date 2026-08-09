    include "rootlib.i"

    section text,code

    xdef _conunder
    xdef _conreverse
    xdef _conbold
    xdef _connormal
    xdef _conclr
    xdef _conputc
    xdef _conputs
    xdef _congetc

_conunder:
    move.l a6,-(sp)
    move.l ROOTLIB_BASE,a6
    jsr CONUNDER(a6)
    move.l (sp)+,a6
    rts

_conreverse:
    move.l a6,-(sp)
    move.l ROOTLIB_BASE,a6
    jsr CONREVERSE(a6)
    move.l (sp)+,a6
    rts

_conbold:
    move.l a6,-(sp)
    move.l ROOTLIB_BASE,a6
    jsr CONBOLD(a6)
    move.l (sp)+,a6
    rts

_connormal:
    move.l a6,-(sp)
    move.l ROOTLIB_BASE,a6
    jsr CONNORMAL(a6)
    move.l (sp)+,a6
    rts

_conclr:
    move.l a6,-(sp)
    move.l ROOTLIB_BASE,a6
    jsr CONCLR(a6)
    move.l (sp)+,a6
    rts

_conputc:
    move.l a6,-(sp)
    move.l ROOTLIB_BASE,a6
    jsr CONPUTC(a6)
    move.l (sp)+,a6
    rts

_conputs:
    move.l a6,-(sp)
    move.l ROOTLIB_BASE,a6
    jsr CONPUTS(a6)
    move.l (sp)+,a6
    rts

_congetc:
    move.l a6,-(sp)
    move.l ROOTLIB_BASE,a6
    jsr CONGETC(a6)
    move.l (sp)+,a6
    rts