START equ $10000
    org START

    incdir ..
    incdir ../storage
    incdir ../lib
    include hardware.i
    include rootlib.i
    include osvars.i

MAX_CMDLINE_LENGTH equ 128
TESTSECTOR equ $800000
    ; Install error handlers
    bsr InstallExceptionHandlers

    bsr MemInit

    lea OSVARS_BASE,a5
    move.l ROOTLIB_BASE,a6 
    move.l #JT_DOS_LIB_BASE,DosLibBase    
        
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
    move.l #READBUFFER_SIZE,d0
    bsr MemAlloc
    move.l d0,ReadBufferPtr

    bsr ClearCommandLine
    lea CommandLine,a4  ; Command line buffer
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
    move.b d0,(a4,d6.w)    
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
    clr.b (a4,d6.w)
    move.b #8,d0
    jsr CONPUTC(a6)
    move.b #32,d0
    jsr CONPUTC(a6)
    move.b #8,d0
    jsr CONPUTC(a6)
    bra .waitForChar

; Check internal command as provided in A0 with the command line provided in A1
CheckInternalCommand:
.compare:
    move.b (a0)+,d0
    beq.s .commandNameEnd
    
    cmp.b (a1)+,d0
    bne.s .noMatch
    bra.s .compare
.commandNameEnd:
    move.b (a1),d0
    beq.s .match
    cmp.b #' ',d0
    bne.s .noMatch
.match:
    moveq #0,d0
    rts
.noMatch:
    moveq #-1,d0
    rts

ParseCommandLine:
    movem.l a2/a3,-(sp)
    bsr.s .parseCommandLine
    movem.l (sp)+,a2/a3
    rts
.parseCommandLine:    
    lea CommandLine,a1
    bsr TrimLeadingSpaces
    tst.b (a1)
    bne.s .cmdLineNotEmpty
    rts
.cmdLineNotEmpty:
    move.l a1,a3
    lea BuiltInCommands,a2
.nextCommand:
    tst.l (a2)
    beq.s .notBuiltInCommand
    move.l (a2),a0
    move.l a3,a1
    bsr CheckInternalCommand
    tst.l d0
    beq.s .internalCommandFound
    lea 4(a2),a2
    bra.s .nextCommand
.notBuiltInCommand:
    lea NotAnExecutable,a1
    jmp CONPUTS(a6)
.internalCommandFound:
    bsr TrimLeadingSpaces
    move.l 4(a2),a2 ; Jump vector
    jsr (a2)
    rts

; Move pointer A1 to first non space character
TrimLeadingSpaces:
    move.b (a1),d0
    beq.s .endOfString
    cmp.b #' ',d0
    bne.s .endOfString
    adda.l #1,a1
    bra.s TrimLeadingSpaces    
.endOfString:
    rts


ClearCommandLine:
    moveq #MAX_CMDLINE_LENGTH/4-1,d7
    lea CommandLine,a0
.clearCmdLine:
    clr.l (a0)+
    dbra d7,.clearCmdLine
    moveq #0,d6     ; Command line position    

    rts


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

InstallExceptionHandlers:
    lea ExceptionAddressError,a0
    move.l a0,$0000000C
    lea ExceptionIllegalInstruction,a0
    move.l a0,$00000010    
    rts


BuiltInCommands:
    dc.l CommandLs,ExecuteLs
    dc.l CommandCd,ExecuteCd
    dc.l CommandCat,ExecuteCat
    dc.l 0,0
CommandLs:
    dc.b "ls",0
CommandCd:
    dc.b "cd",0
CommandCat:
    dc.b "cat",0
DosLoadingMsg:
    dc.b 13,10,"Loading JOFMODORE DOS 1.0...",13,10,0
DirTextMsg:
    dc.b "<DIR>      ",0
NotDirectoryMsg:
    dc.b "Not a directory",13,10,0    
NotAnExecutable:
    dc.b "Not an executable",13,10,0
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
DosLibBase:
    dc.l 0
READBUFFER_SIZE EQU 2048
ReadBufferPtr:
    dc.l 0
JT_DOS_READ_FILE:   jmp FMReadFile
JT_DOS_READ_DIR:    jmp FMReadDir
JT_DOS_CREATE_CTX:  jmp FMCreateContext
JT_DOS_VERSION:     dc.l 1
JT_DOS_LIB_BASE:    


    include exceptions.asm
    include mmc.asm
    include storagedevice.asm
    include partman.asm
    include fat16.asm
    include fileman.asm
    include decimal.asm
    include memman.asm
    include cd.asm
    include ls.asm
    include cat.asm

DecBuffer:
    ds.b 12,0

CommandLine  EQU *
MmcStatus    EQU CommandLine+MAX_CMDLINE_LENGTH
MmcCmdArg    EQU MmcStatus+4
TestPartitionInfo EQU MmcCmdArg+4
DirectoryCtx EQU TestPartitionInfo+32
DirEntry     EQU DirectoryCtx+32
