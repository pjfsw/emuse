    incdir ..
    incdir ../storage
    include hardware.i

UART_RTS_ENABLE_THRESHOLD EQU $c0
UART_RTS_DISABLE_THRESHOLD EQU $e0
GETC equ -52
PUTHEX8 equ -40
PUTHEX16 equ -34
PUTHEX32 equ -28
PUTS equ -22
PUTC equ -16
MAX_CMDLINE_LENGTH equ 128
TESTSECTOR equ $800000

START equ $10000
    org START
    move.l $4.w,a6
    
    bsr InitStorageDevices    
    tst.l d0
    beq.s .storageOk
    move.l d0,d7
    move.l DebugDebug,d0
    bsr printErrorCode
    lea InitStorageErrorMsg(pc),a1
    jsr PUTS(a6)
    move.l d7,d0
    bsr printErrorCode
.storageOk:
    moveq #0,d1
    bsr PrintDir
    move.l #$1b,d1
    bsr PrintDir
    move.l #$1d,d1
    bsr PrintDir
    bsr ClearCommandLine
    lea CommandLine,a5  ; Command line buffer
MainLoop:    
    lea MsgPrompt(pc),a1
    jsr PUTS(a6) ; Puts
.waitForChar:    
    jsr GETC(a6)
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
    jsr PUTC(a6) ; Putc
    bra.s .waitForChar
.lineBreak:
    lea LineBreakMsg,a1
    jsr PUTS(a6)
    lea CommandLine,a1
    jsr PUTS(a6)    
    bsr ParseCommandLine
    bsr ClearCommandLine
    bra MainLoop    
.eraseChar:
    tst.w d6
    beq.s .waitForChar
    subq.w #1,d6
    clr.b (a5,d6.w)
    move.b #8,d0
    jsr PUTC(a6)
    move.b #32,d0
    jsr PUTC(a6)
    move.b #8,d0
    jsr PUTC(a6)
    bra .waitForChar

ParseCommandLine:
    lea LineBreakMsg,a1
    jsr PUTS(a6)
    move.l a5,a0
.nextPath:
    move.b (a0),d0
    bne.s .notDone
    rts
.notDone:
    lea PathOut,a1
    bsr ExtractNextPathElement
    tst.l d0
    bne.s .pathError
    movem.l a0-a1,-(sp)
    move.b #'"',d0
    jsr PUTC(a6)
    lea PathOut,a1
    jsr PUTS(a6)
    move.b #'"',d0
    jsr PUTC(a6)
    lea LineBreakMsg,a1
    jsr PUTS(a6)
    movem.l (sp)+,a0-a1
    bra.s .nextPath
.pathError:
    lea PathErrorMsg,a1
    jsr PUTS(a6)
    lea LineBreakMsg,a1
    jmp PUTS(a6)


ClearCommandLine:
    moveq #MAX_CMDLINE_LENGTH/4-1,d7
    lea CommandLine,a0
.clearCmdLine:
    clr.l (a0)+
    dbra d7,.clearCmdLine
    moveq #0,d6     ; Command line position    

    rts

printErrorCode:
    jsr PUTHEX32(a6)
    lea LineBreakMsg(pc),a1
    jmp PUTS(a6)

MsgPrompt:
    dc.b 13,10,"[/]$ ",0
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
    
; Directory sector in d1
PrintDir:
    lea FMDeviceList,a0
    lea FM_FS_DATA(a0),a0
    move.l a0,d0    ; FAT struct
    ;moveq #0,d1     ; Root directory
    ;move.l #$f98,d1    
    lea SectorBuffer,a0
    lea DirectoryCtx,a1
    bsr FATOpenDir
    tst.l d0
    beq.s .dirCtxOk
    bra printErrorCode
.dirCtxOk:
    lea DirectoryOfMsg,a1
    jsr PUTS(a6)
    lea DirectoryCtx,a2
    lea DirEntry,a3
.nextEntry:    
    move.l a2,a0
    move.l a3,a1
    bsr FATReadDir
    cmp.l #0,d0
    beq.s .endOfDir
    bpl.s .dirEntryOk
    bra printErrorCode
.dirEntryOk:    
    move.l a3,a1
    bsr PrintEightChars
    bsr PrintSpace
    lea 8(a3),a1
    jsr PUTS(a6)
    bsr PrintSpace
    tst.b DIRENT_ATTR(a3)
    beq.s .isFile
    lea DirTextMsg,a1
    jsr PUTS(a6)
.entryDone:
    bsr PrintSpace
    move.l DIRENT_BLOCK(a3),d0
    jsr PUTHEX32(a6)
    lea LineBreakMsg,a1   
    jsr PUTS(a6)
    bra .nextEntry
.isFile:    
    move.l DIRENT_FSIZE(a3),d0
    jsr PUTHEX32(a6)
    bra.s .entryDone
.endOfDir:
    rts    

PrintSpace:
    move.b #' ',d0
    jmp PUTC(a6)

PrintEightChars:
    moveq #7,d7
.nextChar:
    move.b (a1)+,d0
    move.l a1,-(sp)
    jsr PUTC(a6)
    move.l (sp)+,a1
    dbra d7,.nextChar
    rts

PrintTextAndNumber:
    move.l d0,-(sp)
    jsr PUTS(a6)
    move.l (sp)+,d0
    jsr PUTHEX32(a6)
    lea LineBreakMsg(pc),a1
    jmp PUTS(a6)

DirTextMsg:
    dc.b "   <DIR>",0
DirectoryOfMsg:
    dc.b "Directory listing of /:",13,10,0
PartitionStartMsg:
    dc.b "Partition start: $",0
PartitionSizeMsg:
    dc.b "Partition size:  $",0
InitStorageErrorMsg:
    dc.b 13,10,"Boot device initialization error: ",0
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

PathOut:
    blk.b 16,0

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
