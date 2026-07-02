;____________________________________________________________
;
; FAT16 file system
;____________________________________________________________
    rsreset
FAT_PART_ID    rs.l 1
FAT_PART_SIZE  rs.l 1
FAT_FAT1_START rs.l 1
FAT_FAT2_START rs.l 1
FAT_ROOT_START rs.l 1
FAT_DATA_START rs.l 1
FAT_SECT_PER_CLUST rs.b 1
FAT_RESERVED   rs.b 7
FAT_SIZE       rs.b 0

    rsreset
FAT_DIRCTX_FAT_PTR    rs.l 1
FAT_DIRCTX_PART_ID    rs.l 1
; Pointer to 512 byte sector buffer
FAT_DIRCTX_SECBUF_PTR rs.l 1
FAT_DIRCTX_FIRST_SEC  rs.l 1
FAT_DIRCTX_PARENT_SEC rs.l 1
FAT_DIRCTX_CURR_SEC   rs.l 1
FAT_DIRCTX_CURR_ENT   rs.w 1
FAT_DIRCTX_RESERVED   rs.w 3
FAT_DIRCTX_SIZE       rs.b 0

FAT_ERR_INVALID_CLUSTER EQU $81000000
;____________________________________________________________
;
; FATInitPartition:
;
; Initialize partition if it is a valid FAT16 partition
;
; D0: partition number
; A1: 32-byte FAT partition context to fill if successful
;
; Return: D0 = 0: is valid FAT16
;         D0 ! 0: is not valid FAT 16
;____________________________________________________________
FATInitPartition:
    movem.l d2-d7/a5,-(sp)
    bsr.s .fatCheckPartitionInt
    movem.l (sp)+,d2-d7/a5
    rts
.fatCheckPartitionInt:
    move.l d0,FAT_PART_ID(a1)
    move.l a1,-(sp)
    lea SectorBuffer,a5
    move.l a5,a0
    moveq #0,d1
    bsr PMReadSector
    move.l (sp)+,a1

    tst.l d0
    beq.s .readOk
    rts
.readOk:
    bsr.s .checkIfValidFS
    tst.l d0
    beq.s .isValid
    rts
.isValid:
    move.b $0d(a5),FAT_SECT_PER_CLUST(a1)
    lea $0e(a5),a0
    bsr ReadLe16
    move.l d0,d1 ; d1 = reserved sectors    

    ; fat_size -> d2
    lea $16(a5),a0
    bsr ReadLe16
    move.l d0,d2  ; d2 = sectors per FAT

    ; root_entries -> d3
    lea $11(a5),a0
    bsr ReadLe16
    move.l d0,d3  ; d3 = root entries

    ; root_dir_sectors = root_entries * 32 / 512
    ; if you enforce 512 bytes/sector:
    lsl.l #5,d3              ; root_entries * 32
    add.l #511,d3            ; 
    lsr.l #4,d3              ; /512
    lsr.l #5,d3              ; /512
    
    ; fat1_start = reserved
    move.l d1,FAT_FAT1_START(a1)

    ; fat2_start = reserved + fat_size
    move.l d1,d4
    add.l d2,d4
    move.l d4,FAT_FAT2_START(a1)

    ; root_start = reserved + 2 * fat_size
    move.l d2,d4
    add.l d4,d4              ; fat_size * 2
    add.l d1,d4
    move.l d4,FAT_ROOT_START(a1)

    ; data_start = root_start + root_dir_sectors
    add.l d3,d4
    move.l d4,FAT_DATA_START(a1)

    moveq #0,d0
    rts

.checkIfValidFS:
    ; check boot vector signature
    cmp.b #$55,$1fe(a5)
    bne.s .notFat16
    cmp.b #$aa,$1ff(a5)
    bne.s .notFat16
    ; bytes per sector must be 512 for now
    lea $0b(a5),a0
    bsr ReadLe16
    cmp.w #512,d0
    bne.s .notFat16   
    ; sectors per cluster must be nonzero power of two
    move.b $0d(a5),d1
    beq.s .notFat16
    move.b d1,d2
    subq.b #1,d2
    and.b d1,d2
    bne.s .notFat16

    ; reserved sectors must be nonzero
    lea $0E(a5),a0
    bsr ReadLe16
    tst.w d0
    beq.s .notFat16    
    ; FAT count must be nonzero, normally 2
    tst.b $10(a5)
    beq.s .notFat16
    ; FAT16 must have RootEntCnt != 0
    lea $11(a5),a0
    bsr  ReadLe16
    tst.w d0
    beq.s .notFat16    
.isFat16:    
    moveq #0,d0
    rts
.notFat16:
    moveq #1,d0
    rts

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
FATOpenDir:
    movem.l d2-d7/a5,-(sp)
    bsr.s .fatOpenDirInt
    movem.l (sp)+,d2-d7/a5
    rts
.fatOpenDirInt:
    move.l d0,a5    ; The FAT partition context
    move.l FAT_PART_ID(a5),FAT_DIRCTX_PART_ID(a1)    
    move.l d0,FAT_DIRCTX_FAT_PTR(a1)     
    move.l a0,FAT_DIRCTX_SECBUF_PTR(a1)
    tst.l d1
    bne.s .isSubDir  ; TODO Check what d1 is when non-zero !!!
    move.l FAT_ROOT_START(a5),d1    
    bra.s .dirSectorOk
.isSubDir:    
    cmp.l #$2,d1
    blo.s .badCluster
    cmp.l #$ffff,d1
    bhi.s .badCluster
    bsr FATClusterToSectorInt
.dirSectorOk:    
    move.l FAT_PART_ID(a5),d0
    move.l d1,FAT_DIRCTX_FIRST_SEC(a1)
    move.l d1,FAT_DIRCTX_CURR_SEC(a1)
    clr.w FAT_DIRCTX_CURR_ENT(a1)    
    bsr PMReadSector
    rts
.badCluster:
    move.l #FAT_ERR_INVALID_CLUSTER,d0
    rts
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
FATReadDir:
    movem.l d2-d7/a4-a6,-(sp)
    bsr.s .fatReadDirInt
    movem.l (sp)+,d2-d7/a4-a6
    rts
.fatReadDirInt:
    move.l a0,a5
    move.l a1,a4
    moveq #7,d7
.fillEntry:
    clr.l (a1)+
    dbra d7,.fillEntry

    move.l FAT_DIRCTX_SECBUF_PTR(a5),a6
    moveq #0,d1
    move.w FAT_DIRCTX_CURR_ENT(a5),d1 
.checkEntry:    
    cmp.w #512,d1
    bne.s .sectorOk
    ; Read next sector 
    move.l FAT_DIRCTX_PART_ID(a5),d0
    move.l FAT_DIRCTX_CURR_SEC(a5),d1
    addq.l #1,d1
    move.l d1,FAT_DIRCTX_CURR_SEC(a5)
    move.l a6,a0
    bsr PMReadSector
    tst.l d0
    beq.s .nextSectorOk
    rts
.nextSectorOk:
    clr.w FAT_DIRCTX_CURR_ENT(a5)
    moveq #0,d1      
.sectorOk:    
    move.b (a6,d1.w),d0
    beq.s .endOfDirListing
    cmp.b #$e5,d0
    beq.s .findNextEntry
    cmp.b #$0f,d0
    beq.s .findNextEntry
    move.b 11(a6,d1.w),d0
    btst #2,d0
    bne.s .findNextEntry
    btst #3,d0
    beq.s .entryOk
.findNextEntry:
    add.w #32,d1
    bra.s .checkEntry
.entryOk:
    btst #4,d0
    beq.s .attrOk
    move.b #FILEATTR_DIR,DIRENT_ATTR(a4)
.attrOk:
    move.w d1,FAT_DIRCTX_CURR_ENT(a5)
    
    lea 0(a6,d1.w),a6      
    ; Copy first sector
    lea 26(a6),a0
    jsr ReadLe16
    and.l #$ffff,d0
    move.l d0,DIRENT_BLOCK(a4)
    
    ; Now copy file size
    lea 28(a6),a0
    jsr ReadLe32
    move.l d0,DIRENT_FSIZE(a4)    
    
    ; Copy file name
    move.l a4,a1
    moveq #10,d7     
    moveq #0,d1
.copyFilename:
    move.b (a6,d1.l),(a1)+
    addq.l #1,d1
    dbra d7,.copyFilename    
    move.b #0,(a1)+
    add.w #32,FAT_DIRCTX_CURR_ENT(a5)
    moveq #1,d0
    rts
.endOfDirListing;    
    moveq #0,d0
    rts

    ; Convert 16-bit FAT cluster number in D1 to absolute sector
    ; Input: Block in D1, FAT context in A5
    ; Output: Sector in D1
FATClusterToSectorInt:
    and.l #$ffff,d1
    subq.l #2,d1
    moveq #0,d0
    move.b FAT_SECT_PER_CLUST(a5),d0
    mulu d0,d1
    add.l FAT_DATA_START(a5),d1
    rts

    include dirent.i
    include partman.asm
    include littleendian.asm
