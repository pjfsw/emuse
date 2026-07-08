    incdir ..
    incdir ../storage
    incdir ../lib
    include hardware.i
    include rootlib.i

UART_RTS_ENABLE_THRESHOLD EQU $c0
UART_RTS_DISABLE_THRESHOLD EQU $e0
MAX_CMDLINE_LENGTH equ 128
TESTSECTOR equ $800000

START equ $10000
    org START
    bsr MemInit

    move.l $4.w,a6 
    
    lea DosLoadingMsg,a1
    jsr CONPUTS(a6)

    bsr InitStorageDevices    
    tst.l d0
    beq.s .storageOk
    move.l d0,d7
    lea InitStorageErrorMsg(pc),a1
    jsr CONPUTS(a6)
    move.l d7,d0
    bsr PrintErrorCode
    rts
.storageOk:
    bsr ClearCommandLine
    lea CommandLine,a5  ; Command line buffer
MainLoop:    
    lea MsgPrompt(pc),a1
    jsr CONPUTS(a6) ; CONPUTS
.waitForChar:    
    jsr CONGETC(a6)
    tst.l d0
    bmi.s .waitForChar    
    cmp.b #$7f,d0
    beq.s .eraseChar
    cmp.b #13,d0
    beq.s .lineBreak
    cmp.w #MAX_CMDLINE_LENGTH-1,d6
    bhs.s .waitForChar
    cmp.b #32,d0
    blo.s .waitForChar
    cmp.b #127,d0
    bhi.s .waitForChar
    move.b d0,(a5,d6.w)    
    addq.w #1,d6
    jsr CONPUTC(a6) ; CONPUTC
    bra.s .waitForChar
.lineBreak:
    lea LineBreakMsg,a1
    jsr CONPUTS(a6)
    ;lea CommandLine,a1
    ;jsr CONPUTS(a6)    
    bsr ParseCommandLine
    bsr ClearCommandLine
    bra MainLoop    
.eraseChar:
    tst.w d6
    beq.s .waitForChar
    subq.w #1,d6
    clr.b (a5,d6.w)
    move.b #8,d0
    jsr CONPUTC(a6)
    move.b #32,d0
    jsr CONPUTC(a6)
    move.b #8,d0
    jsr CONPUTC(a6)
    bra .waitForChar

ParseCommandLine:
    lea DirEntry,a3
    lea DirectoryCtx,a0
    lea CommandLine,a1    
    bsr FMCreateContext
    tst.l d0
    beq.s .resolveOk
    bra PrintCommandError
.resolveOk:
    lea DirectoryCtx,a0
    btst.b #PATTR_DIR_BIT,PCTX_ATTR(a0)
    bne.s .isDir
    bra ReadFileContents
.isDir:
.nextEntry:    
    lea DirectoryCtx,a0
    move.l a3,a1
    bsr FMReadDir
    cmp.l #0,d0
    beq.s .endOfDir
    bpl.s .dirEntryOk
    bra PrintCommandError
.dirEntryOk:    
    bsr PrintDirEntry
    bra.s .nextEntry
.endOfDir:
    rts
PrintCommandError:
    move.l d0,d7
    lea CommandError,a1
    jsr CONPUTS(a6)
    move.l d7,d0
    bra PrintErrorCode

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

ClearCommandLine:
    moveq #MAX_CMDLINE_LENGTH/4-1,d7
    lea CommandLine,a0
.clearCmdLine:
    clr.l (a0)+
    dbra d7,.clearCmdLine
    moveq #0,d6     ; Command line position    

    rts

ReadFileContents:
    lea DirectoryCtx,a0
    lea TestBuf,a1
    move.l #1024,d0
    bsr FMReadFile
    tst.l d0
    beq.s .readDone
    bpl.s .readOk
    bra PrintErrorCode
.readOk:
    move.l d0,d7
    subq.l #1,d7
    lea TestBuf,a2
.printChar:
    move.b (a2)+,d0
    bsr PrintChar
    dbra d7,.printChar
    bra ReadFileContents
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

PrintErrorCode:
    jsr CONPUTHEX32(a6)
    lea LineBreakMsg(pc),a1
    jmp CONPUTS(a6)

MsgPrompt:
    dc.b "[/]$ ",0
    even

InitStorageDevices:
    bsr FMInit    
    bsr MMCInit  
    move.w d0,MmcStatus
    beq.s .mmcOk
    and.w #$ffff,d0
    rts
.mmcOk:
    lea MmcStorageDevice,a0
    bsr FMRegisterDevice
    rts
    
PrintSpace:
    move.b #' ',d0
    jmp CONPUTC(a6)

PrintEightChars:
    moveq #7,d7
.nextChar:
    move.b (a1)+,d0
    move.l a1,-(sp)
    jsr CONPUTC(a6)
    move.l (sp)+,a1
    dbra d7,.nextChar
    rts

PrintTextAndNumber:
    move.l d0,-(sp)
    jsr CONPUTS(a6)
    move.l (sp)+,d0
    jsr CONPUTHEX32(a6)
    lea LineBreakMsg(pc),a1
    jmp CONPUTS(a6)

DosLoadingMsg:
    dc.b 13,10,"Loading JOFMODORE DOS 1.0...",13,10,0
DirTextMsg:
    dc.b "<DIR>      ",0
NotDirectoryMsg:
    dc.b "Not a directory",13,10,0    
CommandError:
    dc.b "Command return error:",0
InitStorageErrorMsg:
    dc.b 13,10,"Failed to initialize boot device: ",0
PathErrorMsg:
    dc.b "Invalid path",0
LineBreakMsg:
    dc.b 13,10,0    
    even

MmcStorageDevice:
    dc.b "SD"
    dc.l MMCReadSector
    dc.l MMCWriteSector
    blk.w 10,0

    include mmc.asm
    include storagedevice.asm
    include partman.asm
    include fat16.asm
    include fileman.asm
    include decimal.asm
    include memman.asm

DecBuffer:
    dc.b 11,0

SectorBuffer EQU SYSTEM_BSS_BASE+4
CommandLine  EQU *
SDDeviceList EQU CommandLine+MAX_CMDLINE_LENGTH
PMPartList   EQU SDDeviceList+256
MmcStatus    EQU PMPartList+512
MmcCmdArg    EQU MmcStatus+4
TestPartitionInfo EQU MmcCmdArg+4
DirectoryCtx EQU TestPartitionInfo+32
DirEntry     EQU DirectoryCtx+32
FMDeviceList EQU DirEntry+32
DOSLibScratch EQU FMDeviceList+256
TestBuf     EQU $20000