;____________________________________________________________
;
; File System Manager
;
; Abstraction layer containing basic file i/o features
;____________________________________________________________

FM_MAX_DEVICE_COUNT  equ PM_PART_LIMIT
FM_ERR_NO_PARTITIONS_FOUND equ $88010000
FM_ERR_INVALID_PATH  equ $88020000

    rsreset
FM_DEVICE_NAME  rs.b 4
FM_PM_PART_ID   rs.l 1
FM_FS_DATA      rs.b FAT_SIZE
FM_OPEN_DIR     rs.l 1
FM_READ_DIR     rs.l 1
FM_UNPADDED_SIZE rs.b 0
FM_PADDING      rs.b 64-FM_UNPADDED_SIZE
FM_SIZE         rs.b 0
    if FM_UNPADDED_SIZE > 64
        fail "FM structure exceeds 64 bytes"
    endif
    PRINTV FM_SIZE
;____________________________________________________________
;
; Initialize the File system manager
;____________________________________________________________
FMInit:
    move.l d7,-(sp)
    bsr .fmInitInt
    move.l (sp)+,d7
    rts
.fmInitInt:
    bsr SDInit    
    bsr PMInit 
    lea FMDeviceList,a0
    moveq #FM_SIZE*FM_MAX_DEVICE_COUNT/4-1,d7
.nextLong:
    clr.l (a0)+
    dbra d7,.nextLong
    rts

;____________________________________________________________
;
; Register a new device 
;
; A0: Pointer to StorageDeviceStruct
; D0: 0 on success, < 0 on error
;____________________________________________________________
FMRegisterDevice:       
    movem.l d5-d7/a5-a6,-(sp)
    bsr .fmRegisterDeviceInt
    movem.l (sp)+,d5-d7/a5-a6
    rts
.fmRegisterDeviceInt:    
    clr.l DebugDebug
    move.l a0,a6
    bsr SDRegisterDevice         
    cmp.l #0,d0
    bhi.s .registerSDOk    
    move.l #1,DebugDebug
    rts
.registerSDOk:
    bsr PMRegisterDevice
    tst.l d0
    beq.s .registerPMOk
    move.l #2,DebugDebug
    rts
.registerPMOk: 
    bsr PMGetPartitionCount
    tst.l d0
    bne.s .hasPartitions
    move.l #FM_ERR_NO_PARTITIONS_FOUND,d0
    move.l #3,DebugDebug
    rts
.hasPartitions:    
    ; For now just register the first partition
    moveq #0,d0
    lea FMDeviceList,a5
    lea FM_FS_DATA(a5),a1
    bsr FATInitPartition
    tst.l d0
    beq.s .fatOk
    move.l #4,DebugDebug
    rts
.fatOk:    
    ; Setup partition id, i.e. SD0
    move.w SD_ID(a6),FM_DEVICE_NAME(a5)
    move.b d6,d0
    add.b #'0',d0
    move.b d0,2+FM_DEVICE_NAME(a5)
    move.l #0,FM_PM_PART_ID(a5)
    move.l #FATOpenDir,FM_OPEN_DIR(a5)
    move.l #FATReadDir,FM_READ_DIR(a5)   
    moveq #0,d0
    move.l #5,DebugDebug
    rts 

DebugDebug: dc.l 0
     
NormalizeChar:
    cmp.b #32,d0
    blo.s .invalidChar
    cmp.b #127,d0
    bhi.s .invalidChar
    cmp.b #'.',d0
    beq.s .invalidChar
    cmp.b #'\',d0
    beq.s .invalidChar
    cmp.b #'/',d0
    beq.s .invalidChar
    cmp.b #'a',d0
    blo.s .okChar
    cmp.b #'z',d0
    bhi.s .okChar    
    sub.w #32,d0
.okChar:
    rts
.invalidChar:
    moveq #-1,d0
    rts    

; 
; A0: Remaining path input
; A1: Pointer to word-aligned 16 byte buffer where the next path element will be stored
; Return D0: 0 = OK, !0 = Error
ExtractNextPathElement:
    movem.l d7/a2,-(sp)
    bsr.s .extractNextPathElementInt
    movem.l (sp)+,d7/a2
    rts    
.extractNextPathElementInt:    
    clr.l (a1)
    clr.l 4(a1)
    clr.l 8(a1)
    clr.l 12(a1)
    moveq #7,d7    ; Max path element size
    move.l a1,a2
.nextBaseChar:
    move.b (a0)+,d0
    tst.b d0
    beq.s .subPathFound
    cmp.b #'/',d0            
    beq.s .subPathFound
    cmp.b #'.',d0
    beq.s .basePartDone
    bsr NormalizeChar
    tst.l d0
    bmi.s .invalidPath 
    move.b d0,(a1)+
    dbra d7,.nextBaseChar
.invalidPath:    
    move.b (a0)+,d0
    tst.b d0
    beq.s .subPathFound
    cmp.b #'.',d0
    beq.s .parseExtChar    
    move.l #FM_ERR_INVALID_PATH,d0
    rts
.basePartDone:
    move.b #' ',(a1)+
    dbra d7,.basePartDone
.parseExtChar:
    moveq #2,d7
.nextExtChar:
    move.b (a0)+,d0
    tst.b d0
    beq.s .subPathFound
    cmp.b #'/',d0            
    beq.s .subPathFound
    bsr NormalizeChar
    tst.l d0
    bmi.s .invalidPath 
    move.b d0,(a1)+
    dbra d7,.nextExtChar
    move.b (a0)+,d0
    tst.b d0
    beq.s .subPathFound
    cmp.b #'/',d0
    beq.s .subPathFound
    move.l #FM_ERR_INVALID_PATH,d0
    rts
.subPathFound:
    move.l a2,a1
    moveq #10,d7
.zeroToSpace:
    move.b (a1)+,d0
    tst.b d0
    bne.s .notZero
    move.b #' ',-1(a1)
.notZero:    
    dbra d7,.zeroToSpace
    moveq #0,d0
    rts

;____________________________________________________________
;
; Open directory for reading
;
; A0: Pointer to target directory context
; A1: Pointer to path (zero-terminated)
;
; D0: 0 on success, < 0 on error
;____________________________________________________________
FMOpenDir:
    ; For current path element
    ;   Find it in dir
    ;     If not found or file, exit error
    ;     If dir, set current path element

    include fat16.asm
    include partman.asm
    include storagedevice.asm

