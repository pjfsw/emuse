;____________________________________________________________
;
; File System Manager
;
; Abstraction layer containing basic file i/o features
;____________________________________________________________

FM_MAX_DEVICE_COUNT  equ PM_PART_LIMIT
FM_ERR_NO_PARTITIONS_FOUND equ $88010000
FM_ERR_INVALID_PATH  equ $88020000
FM_ERR_PATH_NOT_FOUND equ $88030000

;____________________________________________________________
;
; DOSLibScratch
;____________________________________________________________
    rsreset
DOSINFO_DIRENT  rs.b 32
DOSINFO_PATHENT rs.b 16
DOSINFO_RBUF    rs.b 512

;____________________________________________________________
;
; FMDeviceList
;____________________________________________________________
    rsreset
FM_DEVICE_NAME  rs.b 4
FM_PM_PART_ID   rs.l 1
FM_FS_DATA      rs.b FAT_SIZE
;____________________________________________________________
;
; FATOpenDir
;
; Create a directory context based on a specific dir identifier
;
; D0: FAT context as created by FATInitPartition
; D1: directory first cluster or 0 for root directory
; A0: pointer to target 512 byte sector buffer, must be valid
;     as long as the context is used
; A1: pointer to target 32 byte directory context
;
; Return: D0 = 0: ok
;         D0 ! 0: not ok
;____________________________________________________________
FM_OPEN_DIR     rs.l 1
;____________________________________________________________
;
; FATReadDir
;
; Read next dir entry from dir context
;
; A0: directory context as created by FATOpenDir
; A1: pointer to target 32 byte directory entry
;
; Return: D0 = 0: ok, no more entries (entry is invalid)
;         D0 > 0: ok, end 
;         D0 < 0: not ok
;____________________________________________________________
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

PrintFilename:
    movem.l d7/a1,-(sp)
    moveq #10,d7
.nextChar:
    move.b (a1)+,d0
    cmp.b #31,d0
    bhi.s .ok
    move.b #'X',d0
.ok:
    jsr PUTC(a6)
    dbra d7,.nextChar
    movem.l (sp)+,d7/a1
    rts
;____________________________________________________________
;
; A0 - an 8.3 formatted filename
; A1 - another 8.3 formatted filename
;____________________________________________________________

CompareFilenames:
    movem.l d7/a0-a6,-(sp)
    bsr.s .compareFileNamesInt
    movem.l (sp)+,d7/a0-a6
    rts
 .compareFileNamesInt:    
    move.l a1,a2
    move.l a0,a3
    
    move.l $4.w,a6
    move.b #'"',d0
    jsr PUTC(a6)
    lea .st1,a1
    jsr PUTS(a6)
    move.l a3,a1
    bsr PrintFilename
    move.b #'"',d0
    jsr PUTC(a6)
    lea .st2,a1
    jsr PUTS(a6)
    move.b #'"',d0
    jsr PUTC(a6)
    move.l a2,a1
    bsr PrintFilename
    move.b #'"',d0
    jsr PUTC(a6)
    ; Compare filenames
    move.l a2,a1
    move.l a3,a0
    moveq #10,d7
.compareFilenameChar:
    move.b (a1)+,d0
    cmp.b (a0)+,d0
    bne.s .noMatch
    dbra d7,.compareFilenameChar
    move.b #'!',d0
    jsr PUTC(a6)
    moveq #0,d0    
    rts
.noMatch:
    moveq #-1,d0
    rts
.st1:    
    dc.b 13,10,"A0:",0
.st2:    
    dc.b "A1:",0
    even

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
    movem.l d2/d7/a3-a6,-(sp)
    bsr.s .fmOpenDirInt
    movem.l (sp)+,d2/d7/a3-a6
    rts    
.fmOpenDirInt:  
    lea DOSLibScratch,a3   ; Temporary variables for process - TODO fix this dynamically
    move.l a0,a4    ; Target context
    move.l a1,a5    ; Path
    lea FMDeviceList,a6 ; TODO scan for correct device, for now just first partition
    moveq #0,d2     ; Current directory, for now always root
    move.b (a5),d0
    cmp.b #'/',d0
    bne.s .noLeadingSlash
    lea 1(a5),a5
.noLeadingSlash:
    ; TODO Get current directory
.readCurrentDir:
    lea FM_FS_DATA(a6),a0
    move.l a0,d0
    move.l a4,a1    ; Use target context directly
    move.l d2,d1    ; Directory to scan
    lea DOSINFO_RBUF(a3),a0            
    ;jsr FM_OPEN_DIR(a6)
    jsr FATOpenDir
    tst.l d0
    beq.s .openDirOk
    rts
.openDirOk:
    tst.b (a5)
    bne.s .openNextPath
    rts ; ALL DONE
.openNextPath:      
    move.l a5,a0
    lea DOSINFO_PATHENT(a3),a1
    bsr ExtractNextPathElement
    tst.l d0
    beq.s .scanDirectory
    rts
.scanDirectory:
    move.l a0,a5
.findNextEntry:
    move.l a4,a0
    lea DOSINFO_DIRENT(a3),a1
    ;jsr FM_READ_DIR(a6)
    jsr FATReadDir
    cmp.l #0,d0
    bgt.s .nextEntryOk
    beq.s .pathNotFound
    ; Some other error
    rts
.pathNotFound:
    move.l #FM_ERR_PATH_NOT_FOUND,d0
    rts
.nextEntryOk:
    ;lea DOSINFO_PATHENT(a3),a1
    ;move.l $4.w,a6
    ;jsr PUTS(a6)
    ;rts
    lea DOSINFO_DIRENT(a3),a0
    move.b DIRENT_ATTR(a0),d0
    cmp.b #FILEATTR_DIR,d0
    bne.s .findNextEntry

    lea DOSINFO_PATHENT(a3),a1
    bsr CompareFilenames
    tst.l d0
    bne.s .findNextEntry

    lea DOSINFO_DIRENT(a3),a0
    move.l DIRENT_BLOCK(a0),d2  ; New directory
    bra .readCurrentDir

    include fat16.asm
    include partman.asm
    include storagedevice.asm

