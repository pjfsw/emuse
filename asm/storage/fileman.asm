;____________________________________________________________
;
; File System Manager
;
; Abstraction layer containing basic file i/o features
;____________________________________________________________

FM_OPCODE_JMP_ABSOLUTE equ $4ef9
FM_MAX_DEVICE_COUNT  equ PM_PART_LIMIT

FM_ERR_NO_PARTITIONS_FOUND equ $88010000
FM_ERR_INVALID_PATH        equ $88020000
FM_ERR_PATH_NOT_FOUND      equ $88030000
FM_ERR_NOT_A_DIRECTORY     equ $88040000
FM_ERR_NOT_A_FILE          equ $88050000

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
; FM_CREATE_PATH_CTX
;
; Create a path context based on a specific dir identifier
;
; D0: File system context from initialization
; D1: directory first cluster or 0 for root directory
; A0: pointer to target 512 byte sector buffer, must be valid
;     as long as the context is used
; A1: pointer to target 32 byte directory context
;
; Return: D0 = 0: ok
;         D0 ! 0: not ok
;____________________________________________________________
FM_CREATE_PATH_CTX     rs.w 3
;____________________________________________________________
;
; FM_READ_DIR
;
; Read next dir entry from path context
;
; A0: directory context as created by FATOpenDir
; A1: pointer to target 32 byte directory entry
;
; Return: D0 = 0: ok, no more entries (entry is invalid)
;         D0 > 0: ok, end 
;         D0 < 0: not ok
;____________________________________________________________
FM_READ_DIR     rs.w 3

;____________________________________________________________
;
; FATReadFileSector
;
; Read next sector from a file into a buffer
;
; A0: directory context as created by FATCreatePathContext
; A1: 512 byte sector buffer
;
; Return: D0 = 0: end of file reached
;         D0 > 0: number of bytes read
;         D0 < 0: not ok
;____________________________________________________________
FM_READ_FILE_SECTOR rs.w 3
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
    bsr.s .fmInitInt
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
    bsr.s .fmRegisterDeviceInt
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
    move.w #FM_OPCODE_JMP_ABSOLUTE,d0
    move.w d0,FM_CREATE_PATH_CTX(a5)
    move.l #FATCreatePathContext,2+FM_CREATE_PATH_CTX(a5)
    move.w d0,FM_READ_DIR(a5)
    move.l #FATReadDir,2+FM_READ_DIR(a5)   
    move.w d0,FM_READ_FILE_SECTOR(a5)
    move.l #FATReadFileSector,2+FM_READ_FILE_SECTOR(a5)   
    moveq #0,d0
    move.l #5,DebugDebug
    rts 

DebugDebug: dc.l 0
     
NormalizeChar:
    cmp.b #32,d0
    blo.s .invalidChar
    cmp.b #127,d0
    bhi.s .invalidChar
;    cmp.b #'.',d0
;    beq.s .invalidChar
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
    move.l a1,a2

    ; Special case for "." and ".."
    cmp.b #'.',(a0)
    bne.s .normalCase
    cmp.b #'/',1(a0)
    beq.s .singleDot
    tst.b 1(a0)
    beq.s .singleDot
    cmp.b #'.',1(a0)
    bne.s .normalCase
    cmp.b #'/',2(a0)
    beq.s .doubleDot
    tst.b 2(a0)
    beq.s .doubleDot
    bra.s .normalCase
.singleDot:
    move.b (a0)+,(a1)+
    lea 1(a0),a0
    bra .subPathFound    
.doubleDot:
    move.b (a0)+,(a1)+
    move.b (a0)+,(a1)+
    lea 1(a0),a0
    bra .subPathFound
.normalCase:    
    moveq #7,d7    ; Max path element size
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
    move.b (a0)+,d0
    tst.b d0
    beq.s .subPathFound
    cmp.b #'.',d0
    beq.s .parseExtChar    
.invalidPath:    
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
    jsr CONPUTC(a6)
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
    ;bsr PrintTheFilenames
    moveq #10,d7
.compareFilenameChar:
    move.b (a1)+,d0
    cmp.b (a0)+,d0
    bne.s .noMatch
    dbra d7,.compareFilenameChar
    moveq #0,d0    
    rts
.noMatch:
    moveq #-1,d0
    rts
PrintTheFilenames:
    move.l a1,a2
    move.l a0,a3
    
    move.l $4.w,a6
    move.b #'"',d0
    jsr CONPUTC(a6)
    lea .st1,a1
    jsr CONPUTS(a6)
    move.l a3,a1
    bsr PrintFilename
    move.b #'"',d0
    jsr CONPUTC(a6)
    lea .st2,a1
    jsr CONPUTS(a6)
    move.b #'"',d0
    jsr CONPUTC(a6)
    move.l a2,a1
    bsr PrintFilename
    move.b #'"',d0
    jsr CONPUTC(a6)
    ; Compare filenames
    move.l a2,a1
    move.l a3,a0
    rts

.st1:    
    dc.b 13,10,"A0:",0
.st2:    
    dc.b "A1:",0
    even

;____________________________________________________________
;
; GetCurrentProcess - store pointer to context in A0
;____________________________________________________________
GetCurrentProcess:
    lea DOSLibScratch,a0
    rts

;____________________________________________________________
;
; Create context based on path name
;
; A0: Pointer to target path context
; A1: Pointer to path (zero-terminated)
;
; D0: 0 on success, < 0 on error
;____________________________________________________________
FMCreateContext:
    movem.l d2/d7/a3-a6,-(sp)
    bsr.s .fmCreateContextInt
    movem.l (sp)+,d2/d7/a3-a6
    rts    
.fmCreateContextInt:  
    move.l a0,a4    ; Path context (target)
    move.l a1,a5    ; Path
    bsr GetCurrentProcess
    move.l a0,a3    ; Process context

    lea FMDeviceList,a6 ; TODO scan for correct device, for now just first partition
    move.b #PATTR_DIR,PCTX_ATTR(a4)
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
    jsr FM_CREATE_PATH_CTX(a6)
    tst.l d0
    beq.s .createPathCtxOk
    rts
.createPathCtxOk:
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
    tst.l d2    ; Root directory ?
    bne.s .scanNormally
    lea DOSINFO_PATHENT(a3),a1
    cmp.b #'.',(a1)
    bne.s .scanNormally
    cmp.b #' ',1(a1)
    beq.s .createPathCtxOk
.scanNormally:    
.findNextEntry:
    move.l a4,a0
    lea DOSINFO_DIRENT(a3),a1
    jsr FM_READ_DIR(a6)
    ;jsr FATReadDir
    cmp.l #0,d0
    bgt.s .nextEntryOk
    beq.s .pathNotFound
    ; Some other error
    rts
.pathNotFound:
    move.l #FM_ERR_PATH_NOT_FOUND,d0
    rts
.nextEntryOk:
    lea DOSINFO_DIRENT(a3),a0
    move.b DIRENT_ATTR(a0),PCTX_ATTR(a4)
    move.l DIRENT_FSIZE(a0),PCTX_BYTES_REM(a4)

    lea DOSINFO_PATHENT(a3),a1
    bsr CompareFilenames
    tst.l d0
    bne.s .findNextEntry

    lea DOSINFO_DIRENT(a3),a0
    move.l DIRENT_BLOCK(a0),d2  ; New directory
    bra .readCurrentDir

;____________________________________________________________
;
; Read next directory entry
;
; A0: path context as created by FMCreateContext
; A1: pointer to target 32 byte directory entry
;
; Return: D0 = 0: ok, no more entries (entry is invalid)
;         D0 > 0: ok, end 
;         D0 < 0: not ok
;____________________________________________________________
FMReadDir:
    btst.b #PATTR_DIR_BIT,PCTX_ATTR(a0)
    beq.s .notDirectory
    jmp FATReadDir
.notDirectory:
    move.l #FM_ERR_NOT_A_DIRECTORY,d0
    rts

    include fat16.asm
    include partman.asm
    include storagedevice.asm

;____________________________________________________________
;
; Read from file
;
; A0: path context as created by FMCreateContext
; A1: pointer to target buffer
; D0: number of bytes to read
;
; Return: D0 = 0: end of file
;         D0 > 0: number of bytes read
;         D0 < 0: not ok
;____________________________________________________________
FMReadFile:
    movem.l d5-d7/a3-a6,-(sp)
    bsr.s .fmReadFileInt
    movem.l (sp)+,d5-d7/a3-a6
    rts    
.fmReadFileInt:    
    move.l a0,a4    ; Path context
    btst.b #PATTR_DIR_BIT,PCTX_ATTR(a4)
    beq.s .isFile
    move.l #FM_ERR_NOT_A_FILE,d0
    rts
.isFile:
    bsr GetCurrentProcess
    move.l a0,a3    ; Process context
    lea FMDeviceList,a6 ; TODO scan for correct device, for now just first partition

    move.l a1,a5    ; Read buffer
    move.l d0,d7    ; Bytes to read
    move.l PCTX_BYTES_REM(a4),d0
    cmp.l d0,d7
    bls.s .useD7  ; D7 <= file remaining
    move.l d0,d7   ; use file remaining    
.useD7:
    move.l d7,d6   ; Save value of original bytes to read
    beq .eof       ; EOF, nothing to do
    tst.w PCTX_OFFSET(a4)
    beq.s .aligned
    moveq #-1,d0     ; for now error
    rts
.aligned:
    tst.l d7
    beq.s .eof    
    cmp.l #512,d7
    blo.s .readPartial   
    move.l a4,a0
    move.l a5,a1
    jsr FM_READ_FILE_SECTOR(a6)
    tst.l d0
    beq.s .eof
    bpl.s .readOk    
    rts
.readOk:
    lea 512(a5),a5
    sub.l #512,d7
    bra.s .aligned
.readPartial:
    move.l a4,a0
    lea DOSINFO_RBUF(a3),a1
    jsr FM_READ_FILE_SECTOR(a6)
    tst.l d0
    beq.s .eof
    bpl.s .readPartialOk
    rts
.readPartialOk:
    move.l d7,d0
    add.w d0,PCTX_OFFSET(a4)
    move.w d7,d5
    subq.w #1,d5
    lea DOSINFO_RBUF(a3),a1
.copyBytes:
    move.b (a1)+,(a5)+
    dbra d5,.copyBytes
    moveq #0,d7
.eof:
    move.l d6,d0 ; EOF
    sub.l d7,d0
    sub.l d0,PCTX_BYTES_REM(a4)
    rts