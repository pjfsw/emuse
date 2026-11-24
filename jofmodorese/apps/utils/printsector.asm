; Print contents of buffer stored in A0
PrintSector:
    movem.l d5-d7/a2/a6,-(sp)    
    bsr.s .printSector
    movem.l (sp)+,d5-d7/a2/a6
    rts
.printSector:
    move.l ROOTLIB_BASE,a6
    move.l a0,a2
    ;lea SectorBuffer,a2
    moveq #31,d7
.nextRow:
    moveq #15,d6
    moveq #0,d5
.nextHexCol:
    move.b (a2,d5.w),d0
    jsr CONPUTHEX8(a6)
    move.b #' ',d0
    jsr CONPUTC(a6)
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
    jsr CONPUTC(a6)
    addq.w #1,d5
    dbra d6,.nextAsciiCol

    lea 16(a2),a2
    lea LineBreakMsg(pc),a1
    jsr CONPUTS(a6)
    dbra d7,.nextRow
    rts

;PrintPartitionInfo:
;    moveq #0,d0
;    moveq #0,d1
;    lea TestPartitionInfo,a5
;    move.l a5,a0
;    bsr PMGetPartitionInfo
;    tst.l d0
;    beq.s .partitionInfoOk
;    rts
;.partitionInfoOk:    
;    lea PartitionStartMsg(pc),a1
;    jsr PUTS(a6)
;    move.l PM_PSTART(a5),d0
;    jsr PUTHEX32(a6)
;    lea LineBreakMsg(pc),a1
;    jsr PUTS(a6)
;
;    lea PartitionSizeMsg(pc),a1
;    jsr PUTS(a6)
;    move.l PM_PSIZE(a5),d0
;    jsr PUTHEX32(a6)
;    lea LineBreakMsg(pc),a1
;    jsr PUTS(a6)
;    moveq #0,d0
;    rts