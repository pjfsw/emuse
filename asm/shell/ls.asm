    incdir ../storage
    incdir ../lib
    include dirent.i
    include doslib.i
    include errcode.i

; A1 Command line argument
ExecuteLs:
    movem.l d2/d7/a4-a6,-(sp)
    bsr.s .executeLs
    movem.l (sp)+,d2/d7/a4-a6
    rts
.executeLs:
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
.nextEntry:    
    lea DirectoryCtx,a0
    move.l a3,a1
    jsr DOS_READ_DIR(a4)
    cmp.l #0,d0
    beq.s .endOfDir
    bpl.s .dirEntryOk
    rts
.dirEntryOk:    
    bsr PrintDirEntry
    bra.s .nextEntry
.endOfDir:
    moveq #0,d0
    rts

PrintDirEntry:
    move.l a3,a1
    bsr PrintEightChars
    bsr PrintSpace
    lea 8(a3),a1
    jsr CONPUTS(a6)
    bsr PrintSpace
    tst.b DIRENT_ATTR(a3)
    beq.s .isFile
    lea DirTextMsg,a1
    jsr CONPUTS(a6)
    bra.s .printDate
.isFile:    
    move.l DIRENT_FSIZE(a3),d0
    bsr PrintDecimalLong
.printDate:
    bsr PrintSpace
    bsr PrintSpace
    move.w DIRENT_MOD_DATE(a3),d0
    bsr PrintDate
    bsr PrintSpace
    move.w DIRENT_MOD_TIME(a3),d0
    bsr PrintTime
.printBlock:
    bsr PrintSpace
    bsr PrintSpace
    move.l DIRENT_BLOCK(a3),d0
    jsr CONPUTHEX32(a6)

    lea LineBreakMsg,a1   
    jsr CONPUTS(a6)
    rts
PrintDate:
    move.w DIRENT_MOD_DATE(a3),d2

    rol.w #4,d2
    rol.w #3,d2
    move.w d2,d0
    and.w #127,d0
    add.w #1980,d0
    bsr PrintDateWord
    bsr PrintDash

    rol #4,d2
    move.w d2,d0
    and.w #15,d0
    bsr PrintTwoDigitsDateWord
    bsr PrintDash

    rol #5,d2
    move.w d2,d0
    and.w #31,d0
    bsr PrintTwoDigitsDateWord
    rts
PrintTime:
    move.w DIRENT_MOD_TIME(a3),d2
    rol.w #5,d2
    move.w d2,d0
    and.w #31,d0
    bsr PrintTwoDigitsDateWord
    
    bsr PrintColon
    rol.w #6,d2
    move.w d2,d0
    and.w #63,d0
    bsr PrintTwoDigitsDateWord

    rts
PrintTwoDigitsDateWord:
    lea DecBuffer,a0    
    bsr PrintU16Decimal
    move.l a0,d0
    sub.l #DecBuffer,a0
    moveq #2,d7
    sub.l a0,d7
    bmi.s .print
.pad:
    move.b #'0',d0
    jsr CONPUTC(a6)
    dbra d7,.pad
.print:    
    lea DecBuffer,a1
    jmp CONPUTS(a6)

PrintDateWord:
    lea DecBuffer,a0
    bsr PrintU16Decimal
    lea DecBuffer,a1
    jmp CONPUTS(a6)
PrintDash:
    move.b #'-',d0
    jmp CONPUTC(a6)
PrintColon:
    move.b #':',d0
    jmp CONPUTC(a6)

PrintDecimalWord:
    lea DecBuffer,a0    
    bsr PrintU16Decimal
    lea DecBuffer,a1
    jmp CONPUTS(a6)

PrintDecimalLong:
    lea DecBuffer,a0    
    bsr PrintU32Decimal
    move.l a0,d0
    sub.l #DecBuffer,a0
    moveq #11,d7
    sub.l a0,d7
.pad:
    bsr PrintSpace
    dbra d7,.pad
    lea DecBuffer,a1
    jmp CONPUTS(a6)

DirTextMsg:
    dc.b "<DIR>      ",0
    even

DecBuffer:
    ds.b 12,0
