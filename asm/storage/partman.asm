;____________________________________________________________
;
; Partition manager - Partition abstraction layer
; 
;____________________________________________________________
    rsreset
PM_DEVICE    rs.l 1
PM_INDEX     rs.w 1
PM_TYPE      rs.b 1
PM_FLAGS     rs.b 1
PM_PSTART    rs.l 1
PM_PSIZE     rs.l 1
PM_READ      rs.l 1
PM_WRITE     rs.l 1
PM_RESERVED  rs.l 2
PM_SIZE      rs.b 0

PM_SIZE_SHIFT       equ 5   ; 32 bytes
PM_PART_LIST_SIZE   equ 128
PM_PART_LIMIT       equ 4

PM_ERR_DEVICE_LIST_FULL equ $80100000
PM_ERR_DEVICE_NOT_FOUND equ $80200000
PM_ERR_PARTITION_NOT_FOUND equ $80300000
PM_ERR_DEVICE_IO_ERROR  equ $80f00000

PMInit:
    move.l d7,-(sp)
    bsr.s .pmInitInt
    move.l (sp)+,d7
    rts
.pmInitInt:    
    lea PMPartList,a1
    moveq #PM_PART_LIST_SIZE/4-1,d7
    moveq #0,d0
.loop:    
    move.l d0,(a1)+
    dbra d7,.loop
    rts

    rts

;____________________________________________________________
;
; PMRegisterDevice
;
; Register a storage device adds its valid partitions 
;
; D0: device id to store
; Return: D0 = 0: successful
;         D0 < 0: an error occured
;____________________________________________________________
PMRegisterDevice:
    movem.l d2-d3/d6-d7/a2,-(sp)
    bsr.s .pmRegisterDeviceInt
    movem.l (sp)+,d2-d3/d6-d7/a2
    rts
.pmRegisterDeviceInt:    
    move.l d0,d2
    moveq #0,d1    
    lea SectorBuffer,a0
    bsr SDReadSector
    tst.l d0
    beq.s .readOk
    or.l #PM_ERR_DEVICE_IO_ERROR,d0
    rts
.readOk:    
    lea SectorBuffer,a1
    cmp.w #$55aa,$1fe(a1)    ; Sanity check
    beq.s .isValidMbr
    moveq #0,d0    
    rts    
.isValidMbr:
    moveq #3,d7
    lea $1be(a1),a1
    moveq #0,d3
.nextPartition:
    tst.b $04(a1)
    beq.s .thisPartitionEnd
    moveq #PM_PART_LIMIT-1,d6
    lea PMPartList,a2
.findNextFreeEntry:
    tst.l PM_DEVICE(a2)
    beq.s .foundFreeEntry
    lea PM_SIZE(a2),a2
    dbra d6,.findNextFreeEntry
    move.l #PM_ERR_DEVICE_LIST_FULL,d0
    rts  
.foundFreeEntry:   
    ; Copy stuff     
    move.b $04(a1),PM_TYPE(a2)
    move.l d2,PM_DEVICE(a2)
    move.w d3,PM_INDEX(a2)
    lea $08(a1),a0
    bsr ReadLe32    
    move.l d0,PM_PSTART(a2)
    lea $0c(a1),a0
    bsr ReadLe32
    move.l d0,PM_PSIZE(a2)
        
    addq.l #1,d3
.thisPartitionEnd:    
    lea $10(a1),a1
    dbra d7,.nextPartition    
    moveq #0,d0
    rts

;____________________________________________________________
;
; PMGetPartitionCount
;
; Return the number of registered partitions in D0
;____________________________________________________________
PMGetPartitionCount:
    move.l d7,-(sp)
    bsr.s .pmGetPartitionCountInt
    move.l (sp)+,d7
    rts
.pmGetPartitionCountInt:
    lea PMPartList,a1
    moveq #0,d0
    moveq #PM_PART_LIMIT-1,d7
.nextEntry:
    tst.l PM_DEVICE(a1)
    beq.s .countDone
    addq.l #1,d0
    lea PM_SIZE(a1),a1
    dbra d7,.nextEntry
.countDone:
    rts    

findPartitionFromIndex:
    cmp.l #PM_PART_LIMIT,d0
    bhs.s .notFound          ; unsigned d0 >= limit

    lsl.l #5,d0              ; index * 32
    lea  PMPartList,a1    
    add.l d0,a1

    tst.l PM_DEVICE(a1)
    beq.s .notFound

    moveq #0,d0
    rts
.notFound:
    move.l #PM_ERR_PARTITION_NOT_FOUND,d0
    rts    

;____________________________________________________________
;
; PMGetPartitionInfo
;
; Get partition information
; D0 partition index
; A0 pointer to 32 byte structure to store partition information
; Return: D0: 0 = OK
;____________________________________________________________
PMGetPartitionInfo:
    move.l d7,-(sp)
    bsr.s .pmGetPartitionInfoInt
    move.l (sp)+,d7
    rts
.pmGetPartitionInfoInt:
    bsr findPartitionFromIndex
    tst.l d0
    beq.s .foundPartition
    rts
.foundPartition:
    moveq #PM_SIZE/4-1,d7
.copyPart:
    move.l (a1)+,(a0)+
    dbra d7,.copyPart
    moveq #0,d0
    rts

;____________________________________________________________
;
; PMReadSector
;
; Read sector from partition 

; D0 Partition index
; D1 Sector number relative to partition
; A0: Pointer to 512 byte sector buffer
;
; Return: D0 = 0: OK, D0 != 0: Error 
;____________________________________________________________
PMReadSector:
;    move.l d7,-(sp)
    ;bsr.s .pmReadSectorInt
    ;move.l (sp)+,d7
    ;rts
;.pmReadSectorInt:
    bsr findPartitionFromIndex
    tst.l d0
    beq.s .foundPartition
    rts
.foundPartition:
    add.l PM_PSTART(a1),d1
    move.l PM_DEVICE(a1),d0
    bsr SDReadSector
    tst.l d0
    beq.s .readOk
    or.l #PM_ERR_DEVICE_IO_ERROR,d0
.readOk:
    rts

PMWriteSector:
    rts

    include storagedevice.asm
    include littleendian.asm