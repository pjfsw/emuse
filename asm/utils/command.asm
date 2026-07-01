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
TESTSECTOR equ $800000

START equ $10000
    org START
    move.l $4.w,a6
    
    bsr InitStorageDevices    
    tst.l d0
    beq.s .storageOk
    move.l d0,d7
    lea InitStorageErrorMsg(pc),a1
    jsr PUTS(a6)
    move.l d7,d0
    bsr printErrorCode
.storageOk:
    bsr PrintPartitionInfo
    moveq #0,d0
    moveq #0,d1
    lea SectorBuffer,a0
    bsr PMReadSector
    bsr PrintSector
    bsr PrintFat
MainLoop:    
    lea MsgPrompt(pc),a1
    jsr PUTS(a6) ; Puts
.waitForChar:    
    jsr GETC(a6)
    tst.l d0
    bmi.s .waitForChar
    cmp.b #13,d0
    beq.s .lineBreak
    jsr PUTC(a6) ; Putc
    bra.s .waitForChar
.lineBreak:
    ;bsr TransferTest
    bra MainLoop    

PrintANumberInD0:
    jsr PUTHEX32(a6)

    lea LineBreakMsg,a1
    jsr PUTS(a6)
    rts

printErrorCode:
    jsr PUTHEX32(a6)
    lea LineBreakMsg(pc),a1
    jmp PUTS(a6)

MsgPrompt:
    dc.b 13,10,"[/]$ ",0
    even

InitStorageDevices:
    bsr SDInit    
    bsr PMInit    

    bsr MMCInit  
    move.w d0,MmcStatus
    beq.s .mmcOk
    and.w #$ffff,d0
    rts
.mmcOk:
    lea MmcStorageDevice,a0
    bsr SDRegisterDevice         
    cmp.l #0,d0
    bhi.s .registerSDOk
    moveq #-1,d0
    rts
.registerSDOk:
    bsr PMRegisterDevice
    tst.l d0
    bpl.s .registerPMOk
    moveq #-2,d0
    rts
.registerPMOk: 
    moveq #0,d0
    rts

PrintPartitionInfo:
    moveq #0,d0
    moveq #0,d1
    lea TestPartitionInfo,a5
    move.l a5,a0
    bsr PMGetPartitionInfo
    tst.l d0
    beq.s .partitionInfoOk
    rts
.partitionInfoOk:    
    lea PartitionStartMsg(pc),a1
    jsr PUTS(a6)
    move.l PM_PSTART(a5),d0
    jsr PUTHEX32(a6)
    lea LineBreakMsg(pc),a1
    jsr PUTS(a6)

    lea PartitionSizeMsg(pc),a1
    jsr PUTS(a6)
    move.l PM_PSIZE(a5),d0
    jsr PUTHEX32(a6)
    lea LineBreakMsg(pc),a1
    jsr PUTS(a6)
    moveq #0,d0
    rts
    
PrintFat:
    moveq #0,d0
    lea FatData,a1
    bsr FATInitPartition
    tst.l d0
    beq.s .fatOk
    bra printErrorCode
.fatOk:        
    lea FatData,a2
    move.l a2,d0    ; FAT struct
    moveq #0,d1     ; Root directory
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
    move.l a2,d0
    move.l a3,a1
    bsr FATReadDir
    cmp.l #0,d0
    beq.s .endOfDir
    bpl.s .dirEntryOk
    bra printErrorCode
.dirEntryOk:    
    move.l a3,a1
    jsr PUTS(a6)
    lea LineBreakMsg,a1
    jsr PUTS(a6)
    bra .nextEntry
.endOfDir:
    lea EndOfDirMsg,a1
    jsr PUTS(a6)
    rts    

PrintTextAndNumber:
    move.l d0,-(sp)
    jsr PUTS(a6)
    move.l (sp)+,d0
    jsr PUTHEX32(a6)
    lea LineBreakMsg(pc),a1
    jmp PUTS(a6)


TransferTest:
    moveq #0,d2 ; Sector to load
    move.w #127,d7
.nextSector:        
    moveq #0,d0 ; Force partition 0 for now
    move.l d2,d1
    lea SectorBuffer,a0
    bsr PMReadSector
    tst.l d0
    beq.s .readSectorOk
    jsr PUTHEX32(a6)
    move.b #'R',d0
    jsr PUTC(a6)
    lea LineBreakMsg(pc),a1
    jmp PUTS(a6)
.readSectorOk:
    ;bsr PrintSector
    addq.l #1,d2
    dbra d7,.nextSector
    ;bsr PrintPartition

    rts
 
PrintSector:
    movem.l d5-d7/a2,-(sp)    
    bsr .printSector
    movem.l (sp)+,d5-d7/a2
    rts
.printSector:
    lea SectorBuffer,a2
    moveq #31,d7
.nextRow:
    moveq #15,d6
    moveq #0,d5
.nextHexCol:
    move.b (a2,d5.w),d0
    jsr PUTHEX8(a6)
    move.b #' ',d0
    jsr PUTC(a6)
    addq.w #1,d5
    dbra d6,.nextHexCol

    moveq #15,d6
    moveq #0,d5
.nextAsciiCol:        
    move.b (a2,d5.w),d0
    cmp.b #31,d0
    bhi.s .lowerBoundOk
    move.b #'.',d0
    bra.s .asciiOk
.lowerBoundOk:
    cmp.b #128,d0
    blo.s .asciiOk
    move.b #'.',d0
.asciiOk:
    jsr PUTC(a6)
    addq.w #1,d5
    dbra d6,.nextAsciiCol

    lea 16(a2),a2
    lea LineBreakMsg(pc),a1
    jsr PUTS(a6)
    dbra d7,.nextRow
    rts

DirectoryOfMsg:
    dc.b "Directory listing of /:",13,10,0
EndOfDirMsg:
    dc.b "123456 bytes free.",13,10,0
PartitionStartMsg:
    dc.b "Partition start: $",0
PartitionSizeMsg:
    dc.b "Partition size:  $",0
InitStorageErrorMsg:
    dc.b 13,10,"Boot device initialization error: ",0
LineBreakMsg:
    dc.b 13,10,0    
    even

MmcStorageDevice:
    dc.l "mmc0"
    dc.l MMCReadSector
    dc.l MMCWriteSector
    blk.l 5,0

    include mmc.asm
    include storagedevice.asm
    include partman.asm
    include fat16.asm

SectorBuffer EQU SYSTEM_BSS_BASE+4
SDDeviceList EQU *
PMPartList   EQU SDDeviceList+256
MmcStatus    EQU PMPartList+512
MmcCmdArg    EQU MmcStatus+4
TestPartitionInfo EQU MmcCmdArg+4
FatData      EQU TestPartitionInfo+32
DirectoryCtx EQU FatData+32
DirEntry     EQU DirectoryCtx+32