;____________________________________________________________
;
; FAT16 file system
;
; struct FatDirEntry {
;     char     name[8];
;     char     ext[3];
;     uint8_t  attr;
;     uint8_t  ntReserved;
;     uint8_t  createTimeTenths;
;     uint16_t createTime;
;     uint16_t createDate;
;     uint16_t accessDate;
;     uint16_t firstClusterHigh;   // FAT32 only
;     uint16_t modifyTime;
;     uint16_t modifyDate;
;     uint16_t firstClusterLow;
;     uint32_t fileSize;
; };
; 
; 0x01 Read Only
; 0x02 Hidden
; 0x04 System
; 0x08 Volume Label
; 0x10 Directory
; 0x20 Archive
; 0x40 unused
; 0x80 unused
; 
;____________________________________________________________

    include "fat16.i"

FAT_ERR_INVALID_CLUSTER EQU $81000000

;____________________________________________________________
;
; FATInitPartition:
;
; Initialize partition if it is a valid FAT16 partition
;
; D0: partition number
; A0: pointer to 512 byte sector buffer
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
    move.l a0,a5
    ;lea SectorBuffer,a5
    ;move.l a5,a0
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
; FATCreatePathContext
;
; Create a path context based on partition and first cluster
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
FATCreatePathContext:
    movem.l d2-d7/a5,-(sp)
    bsr.s .fatCreathPathContextInt
    movem.l (sp)+,d2-d7/a5
    rts
.fatCreathPathContextInt:
    move.l d0,a5    ; The FAT partition context
    move.l FAT_PART_ID(a5),PCTX_PART_ID(a1)    
    move.l d0,PCTX_FS_PTR(a1)     
    move.l a0,PCTX_SECBUF_PTR(a1)
    tst.l d1
    bne.s .isSubDir  ; TODO Check what d1 is when non-zero !!!
    clr.l PCTX_CURR_BLOCK(a1)
    move.l FAT_ROOT_START(a5),d1    
    bra.s .dirSectorOk
.isSubDir:    
    cmp.l #$2,d1
    blo.s .badCluster
    cmp.l #$ffff,d1
    bhi.s .badCluster
    move.l d1,PCTX_CURR_BLOCK(a1)
    move.l a5,a0
    bsr FATClusterToSectorInt
.dirSectorOk:    
    move.l FAT_PART_ID(a5),d0
    move.l d1,PCTX_FIRST_SEC(a1)
    move.l d1,PCTX_NEXT_SEC(a1)
    clr.w PCTX_OFFSET(a1)    
    moveq #0,d0
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
; A0: directory context as created by FATCreatePathContext
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
    move.l a0,a5    ; Directory context in A5
    move.l a1,a4    ; Directory entry in A4
    moveq #7,d7
.fillEntry:
    clr.l (a1)+
    dbra d7,.fillEntry

    move.l PCTX_SECBUF_PTR(a5),a6
    moveq #0,d1
    move.w PCTX_OFFSET(a5),d1 
.checkEntry:    
    and.w #$1ff,d1
    bne.s .sectorOk
    ; Read next sector 
    move.l PCTX_PART_ID(a5),d0
    move.l PCTX_NEXT_SEC(a5),d1
    move.l a6,a0
    bsr PMReadSector
    tst.l d0
    beq.s .nextSectorOk
    rts
.nextSectorOk:
    move.l PCTX_NEXT_SEC(a5),d1
    addq.l #1,d1
    move.l d1,PCTX_NEXT_SEC(a5)

    clr.w PCTX_OFFSET(a5)
    moveq #0,d1      
.sectorOk:    
    move.b (a6,d1.w),d0
    beq .endOfDirListing
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
    move.b #PATTR_DIR,DIRENT_ATTR(a4)
.attrOk:
    move.w d1,PCTX_OFFSET(a5)
    
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

    ; Copy modify date
    lea 22(a6),a0
    jsr ReadLe16
    move.w d0,DIRENT_MOD_TIME(a4)

    lea 24(a6),a0
    jsr ReadLe16
    move.w d0,DIRENT_MOD_DATE(a4)
    
    ; Copy file name
    move.l a4,a1
    moveq #10,d7     
    moveq #0,d1
.copyFilename:
    move.b (a6,d1.l),(a1)+
    addq.l #1,d1
    dbra d7,.copyFilename    
    move.b #0,(a1)+
    add.w #32,PCTX_OFFSET(a5)
    moveq #1,d0
    rts
.endOfDirListing;    
    moveq #0,d0
    rts

; Convert 16-bit FAT cluster number in D1 to absolute sector
; Input: Block in D1, FAT context in A0
; Output: Sector in D1
FATClusterToSectorInt:
    and.l #$ffff,d1
    subq.l #2,d1
    moveq #0,d0
    move.b FAT_SECT_PER_CLUST(a0),d0
    mulu d0,d1
    add.l FAT_DATA_START(a0),d1
    rts

;____________________________________________________________
; A0 = filesystem context
; A1 = path context
;____________________________________________________________
FATGetNextCluster:
    movem.l d2-d3/a2,-(sp)
    bsr.s .fatGetNextClusterInt
    movem.l (sp)+,d2-d3/a2
    rts
.fatGetNextClusterInt:
    move.l FAT_FAT1_START(a0),d1
    move.l a1,a2
    move.l PCTX_CURR_BLOCK(a2),d0
    move.l d0,d2
    add.l d2,d2 ; FAT16 byte offset
    move.l d2,d3
    lsr.l #7,d3
    lsr.l #2,d3 ; /512
    add.l d1,d3 ; FAT sector LBA
    and.w #$1fe,d2
    ; Read the sector
    move.l d3,d1
    move.l PCTX_PART_ID(a2),d0
    move.l PCTX_SECBUF_PTR(a2),a0
    bsr PMReadSector
    tst.l d0
    bpl .ok        
    rts
.ok:
    move.l PCTX_SECBUF_PTR(a2),a0
    moveq #0,d0
    move.b 1(a0,d2.w),d0
    lsl.w #8,d0
    move.b 0(a0,d2.w),d0    ; d0 = next cluster
    rts


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
;         D0 > 0: number of bytes read (512)
;         D0 < 0: not ok
;____________________________________________________________
FATReadFileSector:
    movem.l d2-d7/a3-a6,-(sp)
    bsr.s .fatReadFileInt
    movem.l (sp)+,d2-d7/a3-a6
    rts
.fatReadFileInt:
    move.l a0,a5    ; Path context in A5
    move.l a1,a3    ; Save sector buffer

    move.l PCTX_FS_PTR(a5),a4
    moveq #0,d0
    move.b FAT_SECT_PER_CLUST(a4),d0
    add.l PCTX_FIRST_SEC(a5),d0
    move.l PCTX_NEXT_SEC(a5),d1
    cmp.l d0,d1
    blo.s .sameCluster
    ; Need to get new cluster
    move.l a4,a0
    move.l a5,a1
    bsr FATGetNextCluster   
    move.l d0,PCTX_CURR_BLOCK(a5)
    move.l d0,d1
    move.l a4,a0
    bsr FATClusterToSectorInt
    move.l d1,PCTX_FIRST_SEC(a5)
    move.l d1,PCTX_NEXT_SEC(a5)            
.sameCluster:        
    move.l PCTX_NEXT_SEC(a5),d1
    move.l PCTX_PART_ID(a5),d0
    move.l a3,a0
    bsr PMReadSector
    add.l #1,PCTX_NEXT_SEC(a5)   
    move.l #512,d0
    rts

    include dirent.i
    include partman.asm
    include littleendian.asm
