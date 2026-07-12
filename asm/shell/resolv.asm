; A1 Command line argument
ExecuteResolve:
    movem.l d2-d7/a4-a6,-(sp)
    bsr.s .executeResolve
    movem.l (sp)+,d2-d7/a4-a6
    rts
.executeResolve:
    move.l ROOTLIB_BASE,a6
    move.l DosLibBase,a4
    lea DirectoryCtx,a0
    jsr DOS_CREATE_CONTEXT(a4)
    tst.l d0
    beq.s .resolveOk
    rts
.resolveOk:
    lea DirectoryCtx,a0
    move.l PCTX_CURR_BLOCK(a0),d0
    jsr CONPUTHEX32(a6)
    lea LineBreakMsg,a1
    jsr CONPUTS(a6)
    moveq #0,d0
    rts
