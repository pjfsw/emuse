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
    tst.l d0 
    beq.s .loadOk1
    rts
.loadOk1:
    move.l a0,a3    
    ;bsr .debugPrint
    move.l ProcHunkStart(a3),a0
    movem.l a2-a6,-(sp)
    jsr (a0)
    movem.l (sp)+,a2-a6
    move.l d0,d2
    move.l a3,a0
    bsr MemFree
    move.l d2,d0    
    rts
.debugPrint:
    move.l ROOTLIB_BASE,a6
    move.l ProcHunkStart(a3),d0
    bsr .printNum
    move.l 4+ProcHunkStart(a3),d0
    bsr .printNum
    move.l 8+ProcHunkStart(a3),d0
    bsr .printNum
    move.l ProcHunkStart(a3),a0
    move.l #512,d0
    bsr DumpMemory

.printNum:
    jsr CONPUTHEX32(a6)
    move.b #' ',d0
    jmp CONPUTC(a6)

PrintNum:
    jsr CONPUTHEX32(a6)
    lea LineBreakMsg,a1
    jmp CONPUTS(a6)

    incdir "../storage"
    include "dumpmemory.asm"
