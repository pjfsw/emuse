    include "rootlib.i"

    section text,code

    xdef _dosCreateCtx
    xdef _dosReadFile

    macro DOS_OPEN
        move.l a6,-(sp)
        move.l ROOTLIB_BASE,a6
    endm

    macro DOS_CLOSE
        move.l (sp)+,a6
    endm

_dosCreateCtx:
    DOS_OPEN
    jsr DOS_CREATE_CONTEXT(a6)
    DOS_CLOSE
    rts

_dosReadFile:
    DOS_OPEN
    jsr DOS_READ_FILE(a6)
    DOS_CLOSE
    rts
    
