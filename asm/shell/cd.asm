    incdir "../lib"
    incdir "../storage"
    include "dirent.i"
    include "errcode.i"

ExecuteCd:
    movem.l d2/d7/a4-a6,-(sp)
    bsr.s .executeCd
    movem.l (sp)+,d2/d7/a4-a6
    rts
.executeCd:
    tst.b (a1)
    bne.s .pathProvided
    moveq #0,d0
    rts
.pathProvided:
    bsr GetProcDosState
    move.l a0,a5
    move.l ROOTLIB_BASE,a6    
    move.l DosLibBase,a4
    lea DirectoryCtx,a0
    jsr DOS_CREATE_CONTEXT(a4)
    tst.l d0
    beq.s .resolveOk
    rts
.resolveOk:
    lea DirectoryCtx,a0
    btst.b #PATTR_DIR_BIT,PCTX_ATTR(a0)
    bne.s .isDir
    moveq #DOS_ERR_NOT_DIRECTORY,d0
    rts
.isDir:
    move.l PCTX_CURR_BLOCK(a0),DosCurrentDir(a5)
    moveq #0,d0
    rts
