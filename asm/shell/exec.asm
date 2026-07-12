; A1 Command line
ExecuteCommand:
    movem.l d2/d7/a3-a6,-(sp)
    bsr.s .executeCommand
    movem.l (sp)+,d2/d7/a3-a6
    rts
.executeCommand:
    move.l a1,a3
.findCommandEnd:    
    move.b (a1),d0
    beq.s .foundCommandEnd
    cmp.b #' ',d0
    beq.s .foundSpace
    lea 1(a1),a1
    bra.s .findCommandEnd
.foundSpace:
    clr.b (a1)
.foundCommandEnd:
    move.l DosLibBase,a4
    lea DirectoryCtx,a0
    move.l a3,a1
    jsr DOS_CREATE_CONTEXT(a4)
    tst.l d0
    beq.s .resolveOk
    rts
.resolveOk:
    lea DirectoryCtx,a0
    move.l DosLibBase,a6
    jmp DOS_LOAD_EXE(a6)

    incdir "../storage"
