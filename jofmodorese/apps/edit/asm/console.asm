    include "rootlib.i"

    section text,code

    xdef _conclrline
    xdef _conclreol
    xdef _consetcrs
    xdef _concrsright
    xdef _concrsleft
    xdef _concrsdown
    xdef _concrsup
    xdef _conunder
    xdef _conreverse
    xdef _conbold
    xdef _connormal
    xdef _conclr
    xdef _conputhex32
    xdef _conputc
    xdef _conputs
    xdef _congetc

_conclrline:
    move.l a6,-(sp)
    move.l ROOTLIB_BASE,a6
    jsr CONCLRLINE(a6)
    move.l (sp)+,a6
    rts

_conclreol:
    move.l a6,-(sp)
    move.l ROOTLIB_BASE,a6
    jsr CONCLREOL(a6)
    move.l (sp)+,a6
    rts

_consetcrs:
    move.l a6,-(sp)
    move.l ROOTLIB_BASE,a6
    jsr CONSETCRS(a6)
    move.l (sp)+,a6
    rts

_concrsdown:
    move.l a6,-(sp)
    move.l ROOTLIB_BASE,a6
    jsr CONCRSDOWN(a6)
    move.l (sp)+,a6
    rts

_concrsup:
    move.l a6,-(sp)
    move.l ROOTLIB_BASE,a6
    jsr CONCRSUP(a6)
    move.l (sp)+,a6
    rts

_concrsleft:
    move.l a6,-(sp)
    move.l ROOTLIB_BASE,a6
    jsr CONCRSLEFT(a6)
    move.l (sp)+,a6
    rts

_concrsright:
    move.l a6,-(sp)
    move.l ROOTLIB_BASE,a6
    jsr CONCRSRIGHT(a6)
    move.l (sp)+,a6
    rts

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

_conputhex32:
    move.l a6,-(sp)
    move.l ROOTLIB_BASE,a6
    jsr CONPUTHEX32(a6)
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