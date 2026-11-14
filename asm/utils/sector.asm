    incdir "../include"
    include "rootlib.i"

START equ $1000
    org START

    move.l ROOTLIB_BASE,a6
    lea WelcomeMsg(pc),a1
    jsr CONPUTS(a6)

    bsr SelectPartition
    tst.l d0
    bpl.s PartitionSelected
    moveq #0,d0
    rts
PartitionSelected:
    lea PartitionIndex(pc),a0
    move.l d0,(a0)

    lea CurrentSector(pc),a5
Main:
    lea CurrentMsg(pc),a1
    jsr CONPUTS(a6)
    move.l (a5),d0
    jsr CONPUTHEX32(a6)
    lea LineBreakMsg(pc),a1
    jsr CONPUTS(a6)

    move.l PartitionIndex(pc),d0
    move.l (a5),d1
    lea SectorBuffer(pc),a0
    jsr DOS_READ_PART_SECTOR(a6)

    lea SectorBuffer(pc),a0
    bsr PrintSector

ShowPrompt:
    lea PromptMsg(pc),a1
    jsr CONPUTS(a6)
WaitForCommand:
    bsr.s WaitKey
    moveq #1,d1
    cmp.b #'n',d0
    beq.s NextSector
    cmp.b #'N',d0
    beq.s NextSector16    
    cmp.b #'p',d0
    beq.s PrevSector
    cmp.b #'P',d0
    beq.s PrevSector16
    cmp.b #3,d0
    beq.s CopySector
    cmp.b #22,d0
    beq.s PasteSector
    cmp.b #'q',d0    
    bne.s WaitForCommand
    moveq #0,d0
    rts

WaitKey:
    jsr CONGETC(a6)
    tst.l d0
    bmi.s WaitKey
    rts

NextSector16:
    moveq #16,d1
NextSector:
    add.l d1,(a5)
    bra.s Main

PrevSector16:
    moveq #16,d1
PrevSector:
    move.l (a5),d0
    sub.l d1,d0
    bpl.s .prevSectorOk
    moveq #0,d0
.prevSectorOk:
    cmp.l (a5),d0        
    beq.s WaitForCommand
    move.l d0,(a5)
    bra Main    

CopySector:
    lea Clipboard(pc),a1
    lea SectorBuffer(pc),a0
    moveq #127,d7
.copyData:
    move.l (a0)+,(a1)+
    dbra d7,.copyData
    lea CopyMsg(pc),a1
    jsr CONPUTS(a6)
    bra ShowPrompt

PasteSector:
    lea ConfirmMsg(pc),a1
    jsr CONPUTS(a6)
    bsr WaitKey
    cmp.b #'y',d0
    beq.s .pasteSectorConfirmed
    lea AbortedMsg(pc),a1
    jsr CONPUTS(a6)
    bra ShowPrompt
.pasteSectorConfirmed:
    move.l PartitionIndex(pc),d0
    move.l (a5),d1
    lea Clipboard(pc),a0
    jsr DOS_WRITE_PART_SECTOR(a6)
    bra Main

;____________________________________________________________
;
; Print the partition list and let the user pick a partition
; Output: D0: selected partition index, or -1 if aborted
;____________________________________________________________
SelectPartition:
    movem.l d6-d7/a2,-(sp)
    bsr.s .selectPartition
    movem.l (sp)+,d6-d7/a2
    rts
.selectPartition:
    jsr DOS_GET_PART_COUNT(a6)
    move.l d0,d7            ; d7 = number of partitions
    bne.s .hasPartitions
    lea NoPartitionsMsg(pc),a1
    jsr CONPUTS(a6)
    moveq #-1,d0
    rts
.hasPartitions:
    lea PartHeaderMsg(pc),a1
    jsr CONPUTS(a6)
    moveq #0,d6             ; d6 = current partition index
.nextPart:
    move.l d6,d0
    lea PartitionInfo(pc),a2
    move.l a2,a0
    jsr DOS_GET_PART_INFO(a6)

    move.b #'0',d0
    add.b d6,d0
    jsr CONPUTC(a6)
    lea PartSepMsg(pc),a1
    jsr CONPUTS(a6)
    lea PM_NAME(a2),a1
    jsr CONPUTS(a6)
    lea PartSepMsg(pc),a1
    jsr CONPUTS(a6)
    move.l PM_PSTART(a2),d0
    jsr CONPUTHEX32(a6)
    lea PartSepMsg(pc),a1
    jsr CONPUTS(a6)
    move.l PM_PSIZE(a2),d0
    jsr CONPUTHEX32(a6)
    lea LineBreakMsg(pc),a1
    jsr CONPUTS(a6)

    addq.l #1,d6
    cmp.l d7,d6
    blo.s .nextPart

    lea SelectMsg(pc),a1
    jsr CONPUTS(a6)
    move.b #'0'-1,d0
    add.b d7,d0
    jsr CONPUTC(a6)         ; highest valid index
    lea SelectEndMsg(pc),a1
    jsr CONPUTS(a6)
.waitPartKey:
    bsr WaitKey
    cmp.b #'q',d0
    beq.s .aborted
    sub.b #'0',d0
    bcs.s .waitPartKey      ; below '0'
    cmp.b d7,d0
    bhs.s .waitPartKey      ; above highest index
    and.l #$ff,d0
    move.l d0,d6
    add.b #'0',d0
    jsr CONPUTC(a6)         ; echo selection
    move.l d6,d0
    rts
.aborted:
    moveq #-1,d0
    rts

    include "printsector.asm"

LineBreakMsg:
    dc.b 13,10,0    
WelcomeMsg:
    dc.b 13,10,"Sector Utility v0.1 by Johan Fransson",0        
CurrentMsg:
    dc.b 13,10,10,"Sector: ",0
PromptMsg:
    dc.b 13,10,"[n]ext [N]ext 16 [p]revious [P]revious 16"
    dc.b 13,10,"[CTRL+C] Copy sector [CTRL+V] paste sector"
    dc.b 13,10,"[Q]uit: ",0
CopyMsg:
    dc.b 13,10,10,"Sector copied to clipboard!",13,10,0    
ConfirmMsg:
    dc.b 13,10,10,"This will overwrite current sector! Press [y] to confirm:",0    
AbortedMsg:
    dc.b " operation aborted",13,10,0
NoPartitionsMsg:
    dc.b 13,10,"No partitions found!",13,10,0
PartHeaderMsg:
    dc.b 13,10,10,"Nr  Part  Start      Size       (sectors, hex)",13,10,0
PartSepMsg:
    dc.b "   ",0
SelectMsg:
    dc.b 13,10,"Select partition [0-",0
SelectEndMsg:
    dc.b "] or [q]uit: ",0
    even
CurrentSector:
    dc.l 0
PartitionIndex:
    dc.l 0
PartitionInfo:
    blk.b PM_SIZEOF,0
SectorBuffer:
    blk.b 512,0
Clipboard:
    blk.b 512,0    