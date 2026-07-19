; A1 Command line
ExecutePart:
    movem.l d6-d7/a2/a6,-(sp)
    bsr.s .executePart
    movem.l (sp)+,d6-d7/a2/a6
    rts
.executePart:
    move.l ROOTLIB_BASE,a6
    lea .hdrMsg(pc),a1
    jsr CONPUTS(a6)

    bsr PMGetPartitionCount
    move.l d0,d7
    moveq #0,d6
    subq.w #1,d7
    bpl.s .nextPart
    moveq #0,d0
    rts
.nextPart:
    
    move.l d6,d0
    lea ShellPartitionInfo,a0
    bsr PMGetPartitionInfo    
    lea ShellPartitionInfo,a2
    
    lea PM_NAME(a2),a1
    jsr CONPUTS(a6)   
   
    move.l PM_PSTART(a2),d0
    bsr PrintDecimalLong

    bsr.s .printSpace
    bsr.s .printSpace

    move.l PM_PSIZE(a2),d0
    lsr.l #6,d0
    lsr.l #5,d0
    bsr PrintDecimalWord

    lea .MBMsg(pc),a1
    jsr CONPUTS(a6)

    lea .unsupportedMsg(pc),a1
    cmp.b #$0e,PM_TYPE(a2)
    bne.s .notRecognized
    lea .fat16Msg(pc),a1
.notRecognized:
    jsr CONPUTS(a6)

    lea LineBreakMsg,a1
    jsr CONPUTS(a6)
    
    addq.l #1,d6
    dbra d7,.nextPart
    moveq #0,d0
    rts
.printSpace:
    move.b #' ',d0
    jmp CONPUTC(a6)

.hdrMsg:
    dc.b "Id       Start   Size  Type",13,10,0
.MBMsg:
    dc.b "M  ",0    
.fat16Msg:
    dc.b "FAT16(LBA) ",0
.unsupportedMsg:
    dc.b "Unsupported",0
    even