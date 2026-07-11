    incdir ../storage
    incdir ../lib
    include dirent.i
    include doslib.i

; A1 Command line
ExecuteCat:
    movem.l d2/d7/a4-a6,-(sp)
    bsr.s .executeCat
    movem.l (sp)+,d2/d7/a4-a6
    rts
.executeCat:
    tst.b (a1)
    bne.s .hasArgument
    rts
.hasArgument:
    move.l ROOTLIB_BASE,a6
    move.l #READBUFFER_SIZE,d2    ; Read buffer size
    move.l ReadBufferPtr,a5     ; Read buffer
    move.l DosLibBase,a4
    lea DirectoryCtx,a0
    jsr DOS_CREATE_CONTEXT(a4)
    tst.l d0
    beq.s .resolveOk
    rts
.resolveOk:
    lea DirectoryCtx,a0
    btst.b #PATTR_DIR_BIT,PCTX_ATTR(a0)
    beq.s .readFileContents
    moveq #-1,d0
    rts
.readFileContents:
    lea DirectoryCtx,a0
    move.l a5,a1
    move.l d2,d0
    jsr DOS_READ_FILE(a4)
    tst.l d0
    beq.s .readDone
    bpl.s .readOk
    rts
.readOk:
    move.l d0,d7
    subq.l #1,d7
    move.l a5,a2
.printChar:
    move.b (a2)+,d0
    bsr PrintChar
    dbra d7,.printChar
    bra .readFileContents
.readDone:
    rts

PrintChar:
    cmp.b #10,d0
    beq.s .printLineBreak
    cmp.b #32,d0
    blo.s .printSpace
    jmp CONPUTC(a6)
.printLineBreak:
    lea LineBreakMsg,a1
    jmp CONPUTS(a6)
.printSpace:
    move.b #' ',d0
    jmp CONPUTC(a6) 
