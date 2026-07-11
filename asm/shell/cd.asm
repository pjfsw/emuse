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
    clr.b 1+DosCurDirName(a5)
    move.l PCTX_CURR_BLOCK(a0),DosCurrentDir(a5)
    beq.s .isRoot
    move.b #'?',DosCurDirName(a5)

    lea DirectoryCtx,a0
    lea .parentDir(pc),a1
    jsr DOS_CREATE_CONTEXT(a4)
    beq.s .readNextDirEntry
    bra.s .done
.readNextDirEntry:
    lea DirectoryCtx,a0
    lea DirEntry,a1
    jsr DOS_READ_DIR(a4)
    cmp.l #0,d0
    bmi.s .done
    beq.s .done
    lea DirEntry,a0
    move.l DIRENT_BLOCK(a0),d0
    cmp.l DosCurrentDir(a5),d0
    bne.s .readNextDirEntry
    moveq #10,d7
    lea DIRENT_NAME(a0),a0
    lea DosCurDirName(a5),a1
.copyChar:
    move.b (a0)+,(a1)+
    dbra d7,.copyChar
.fillZero:
    move.b -(a1),d0
    cmp.b #' ',d0
    bne.s .done
    clr.b (a1)
    bra.s .fillZero
.isRoot:
    move.b #'/',DosCurDirName(a5)
.done:
    moveq #0,d0
    rts
.parentDir: dc.b "..",0
    even