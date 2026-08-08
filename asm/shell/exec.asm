; A1 Command line
ExecuteCommand:
    movem.l d2/d7/a3-a6,-(sp)
    bsr.s .executeCommand
    movem.l (sp)+,d2/d7/a3-a6
    rts
.executeCommand:
    lea ResolvedCmd(pc),a3
    moveq #0,d2 ;  Tracker of dot (extension)
    ;move.l a1,a3
.findCommandEnd:    
    move.b (a1),d0
    beq.s .foundCommandEnd
    cmp.b #' ',d0
    beq.s .foundCommandEnd
    cmp.b #'.',d0
    bne.s .checkSlash
    moveq #1,d2
.checkSlash:    
    cmp.b #'/',d0
    bne.s .copyChar
    moveq #0,d2
.copyChar:
    move.b d0,(a3)+
    lea 1(a1),a1
    bra.s .findCommandEnd
.foundCommandEnd:
    tst.b d2
    bne.s .hasDot
    move.b #'.',(a3)+
    move.b #'e',(a3)+
    move.b #'x',(a3)+
    move.b #'e',(a3)+
.hasDot:
    ; a1 now points to space if argument is present
    clr.b (a3)
    lea ResolvedCmd(pc),a3
    move.l DosLibBase(pc),a4
    lea DirectoryCtx(pc),a0
    move.l a3,a1
    jsr DOS_CREATE_CONTEXT(a4)
    tst.l d0
    beq.s .resolveOk
    rts
.resolveOk:
    lea DirectoryCtx(pc),a0
    move.l DosLibBase(pc),a6
    jsr DOS_LOAD_EXE(a6)
    tst.l d0 
    beq.s .loadOk1
    rts
.loadOk1:
    move.l a0,a3    
;    bsr .debugPrint
    move.l ProcHunkStart(a3),a0
    movem.l a2-a6,-(sp)
    jsr (a0)
    movem.l (sp)+,a2-a6
    move.l d0,d2
    move.l a3,a0
    move.l ROOTLIB_BASE,a6
    jsr MEMFREE(a6)
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
    lea LineBreakMsg(pc),a1
    jmp CONPUTS(a6)

    incdir "../storage"
    include "dumpmemory.asm"
