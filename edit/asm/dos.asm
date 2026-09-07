    include "rootlib.i"
    include "doslib.i"

    section text,code

    xdef _dosCreateCtx
    xdef _dosReadFile

    macro DOS_OPEN
        movem.l d2/a5/a6,-(sp)
        bsr.s openDos
    endm

    macro DOS_CLOSE
        bsr.s closeDos
        movem.l (sp)+,d2/a5/a6
    endm

openDos:
    movem.l d0/a0-a1,-(sp)
    move.l ROOTLIB_BASE,a6
    move.l #DOS_LIB_ID,d0
    moveq #1,d1
    jsr LIBOPEN(a6)
    move.l d0,a5
    movem.l (sp)+,d0/a0-a1
    rts

closeDos:
    move.l d0,d2
    move.l #DOS_LIB_ID,d0
    jsr LIBCLOSE(a6)
    move.l d2,d0
    rts

_dosCreateCtx:
    DOS_OPEN
    jsr DOS_CREATE_CONTEXT(a5)
    DOS_CLOSE
    rts

_dosReadFile:
    DOS_OPEN
    jsr DOS_READ_FILE(a5)
    DOS_CLOSE
    rts
    
