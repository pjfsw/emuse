    incdir "../include"
    include "rootlib.i"

START equ $1000
    org START

    move.l ROOTLIB_BASE,a6
    lea WelcomeMsg(pc),a1
    jsr CONPUTS(a6)

    lea CurrentSector(pc),a5
Main:
    lea CurrentMsg(pc),a1
    jsr CONPUTS(a6)
    move.l (a5),d0
    jsr CONPUTHEX32(a6)
    lea LineBreakMsg(pc),a1
    jsr CONPUTS(a6)

    moveq #1,d0
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
    moveq #1,d0
    move.l (a5),d1    
    lea Clipboard(pc),a0
    jsr DOS_WRITE_PART_SECTOR(a6)
    bra Main
    
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
    even    
CurrentSector:
    dc.l 0    
SectorBuffer:
    blk.b 512,0
Clipboard:
    blk.b 512,0    