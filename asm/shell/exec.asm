    incdir "../include"
    include "psb.i"
    include "errcode.i"

; A1 Command line
ExecuteCommand:
    movem.l d2/d7/a2-a6,-(sp)
    bsr.s .executeCommand
    movem.l (sp)+,d2/d7/a2-a6
    rts
.executeCommand:
    lea ResolvedCmd(pc),a0
    bsr ExecAddExtensionIfNeeded      
    move.l a1,a2    ; Save argument for later
    move.l ROOTLIB_BASE,a6
    move.l #PsbSizeOf,d0
    jsr MEMALLOC(a6)
    tst.l d0
    bne.s .allocOk
    moveq #DOS_ERR_OUT_OF_MEMORY,d0
    rts
.allocOk:    
    move.l a2,a1    ; Transfer argument to A1
    move.l d0,a2    ; PSB pointer
    move.l a2,-(sp)
    bsr.s ExecPreparePsb
    bsr.s ExecuteCommand2
    move.l (sp)+,a0
    move.l d0,d2    ; Save error code
    jsr MEMFREE(a6) ; Free process startup header
    move.l d2,d0
    rts

ExecPreparePsb:
    moveq #PsbSizeOf/2-1,d7       
    move.l a2,a0
.clearPsb:
    clr.w (a0)+
    dbra d7,.clearPsb
.checkArgStart:
    move.b (a1),d0
    beq.s .copyArgDone
    cmp.b #' ',d0
    bne.s .foundArgStart
    lea 1(a1),a1
    bra.s .checkArgStart
.foundArgStart:    
    lea PsbArg(a2),a0
    moveq #PSB_ARG_LENGTH-1,d7
.copyArgByte:
    move.b (a1)+,d0
    beq.s .copyArgDone
    move.b d0,(a0)+    
    dbra d7,.copyArgByte
.copyArgDone:
    rts
   
ExecuteCommand2:
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
    jsr DOS_LOAD_PROCESS(a6)
    tst.l d0 
    beq.s .loadOk1
    rts
.loadOk1:
    move.l a0,a3    
;    bsr .debugPrint
    move.l ProcHunkStart(a3),a0
    movem.l a2-a6,-(sp)
    move.l a2,a1        ; Pointer to PSB in A1
    jsr (a0)
    movem.l (sp)+,a2-a6
    move.l d0,d2
    move.l a3,a0
    move.l ROOTLIB_BASE,a6
    jsr MEMFREE(a6)
    move.l d2,d0    
    rts

; Add .EXE if missing from commandline
; Input: A0: Target buffer to store resolved command
; A1: Raw command line
; Output: A1: contains the pointer to the remaining argument
ExecAddExtensionIfNeeded:
    moveq #0,d2 ;  Tracker of dot (extension)
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
    move.b d0,(a0)+
    lea 1(a1),a1
    bra.s .findCommandEnd
.foundCommandEnd:
    tst.b d2
    bne.s .hasDot
    move.b #'.',(a0)+
    move.b #'e',(a0)+
    move.b #'x',(a0)+
    move.b #'e',(a0)+
.hasDot:
    clr.b (a0)
    rts



    incdir "../storage"
    include "dumpmemory.asm"
