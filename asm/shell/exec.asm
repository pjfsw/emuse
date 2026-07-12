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
    jsr DOS_LOAD_EXE(a6)
    move.l a0,a3    
    move.l ROOTLIB_BASE,a6

    move.w ProcHunkCount(a3),d7
    subq.w #1,d7
    moveq #0,d2
.printHunks:
    move.l ProcHunkStart(a3,d2.w),d0
    bsr PrintNum

    move.l ProcHunkSize(a3,d2.w),d0
    bsr PrintNum

    addq.w #4,d2
    dbra d7,.printHunks
    moveq #0,d0
    rts

PrintNum:
    jsr CONPUTHEX32(a6)
    lea LineBreakMsg,a1
    jmp CONPUTS(a6)

    incdir "../storage"
