    incdir "../lib"
    include "dirent.i"
    include "errcode.i"

ExecuteCd:
    movem.l d2/d7/a2-a6,-(sp)
    bsr.s .executeCd
    movem.l (sp)+,d2/d7/a2-a6
    rts
.executeCd:
    tst.b (a1)
    bne.s .pathProvided
    moveq #0,d0
    rts
.pathProvided:
    move.l ROOTLIB_BASE,a6    
    lea CurrentDir,a5
    move.l DosLibBase,a4
    lea DirectoryCtx,a3
    lea DirEntry,a2
    move.l a3,a0
    jsr DOS_CREATE_CONTEXT(a4)
    tst.l d0
    beq.s .resolveOk
    rts
.resolveOk:
    move.l a3,a0
    jsr DOS_CHANGE_DIR(a4)
    tst.l d0
    beq.s .chdirOk
    rts
.chdirOk:    
    move.l PCTX_CURR_BLOCK(a3),d2
    clr.b 1(a5)
    tst.l d2
    beq.s .isRoot
    move.b #'?',(a5)

    move.l a3,a0
    lea .parentDir(pc),a1
    jsr DOS_CREATE_CONTEXT(a4)
    beq.s .readNextDirEntry
    bra.s .done
.readNextDirEntry:
    move.l a3,a0
    move.l a2,a1
    jsr DOS_READ_DIR(a4)
    cmp.l #0,d0
    bmi.s .done
    beq.s .done
    move.l DIRENT_BLOCK(a2),d0
    cmp.l d2,d0
    bne.s .readNextDirEntry
    moveq #10,d7
    lea DIRENT_NAME(a2),a0
    move.l a5,a1
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
    move.b #'/',(a5)
.done:
    moveq #0,d0
    rts
.parentDir: dc.b "..",0
    even