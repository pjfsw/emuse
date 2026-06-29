;____________________________________________________________
;
; FAT16 file system
;____________________________________________________________
    rsreset
FAT_FAT1_START rs.l 1
FAT_FAT2_START rs.l 1
FAT_ROOT_START rs.l 1
FAT_DATA_START rs.l 1
FAT_SIZE       rs.l 1
FAT_SECT_PER_CLUST rs.b 1


;
; FAT16CheckPartition:
;
; Register partition is a valid FAT16 partition
;
; D0: partition number
; A1: 32-byte struct to fill if successfuly
;
; Return: D0 = 0: is valid FAT16
;         D0 ! 0: is not valid FAT 16
;____________________________________________________________
FATRegisterPartition:
    movem.l d2-d7/a5,-(sp)
    bsr.s .fatCheckPartitionInt
    movem.l (sp)+,d2-d7/a5
    rts
.fatCheckPartitionInt:
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

    include partman.asm
    include littleendian.asm

