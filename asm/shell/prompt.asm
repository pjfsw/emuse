;
; Display prompt and read data
; A0 - pointer to shell data struct
Prompt:
    movem.l a4-a6/d2-d3/d6-d7,-(sp)
    bsr.s .prompt
    movem.l (sp)+,a4-a6/d2-d3/d6-d7
    rts
.prompt:
    move.l a0,a5    ; Prompt variable pointer in a5
    lea ShellInputBuffer(a5),a4 ; Prompt buffer in a4
    move.l ROOTLIB_BASE,a6
    move.l a4,a0

    ; Clear prompt buffer
    moveq #MAX_CMDLINE_LENGTH/4-1,d7
.clrPrompt:
    clr.l (a0)+
    dbra d7,.clrPrompt

    bsr PrintPrompt

    ; D7 = Length
    ; D6 = position
    clr.l d6    
    clr.l d7
.waitForChar:
    bsr PromptWaitKey
    cmp.b #27,d0
    beq .readEscapeSequence
    cmp.b #$7f,d0
    beq.s .eraseChar
    cmp.b #13,d0
    beq.s .lineBreak
    cmp.w #MAX_CMDLINE_LENGTH-1,d7
    bhs.s .waitForChar
    cmp.b #32,d0
    blo.s .waitForChar
    cmp.b #127,d0
    bhi.s .waitForChar
    cmp.w d7,d6
    bne.s .insertChar
    bsr.s .appendChar
    bra.s .waitForChar
.insertChar:    
    ; Insert the character
    move.w d7,d1
    sub.w d6,d1
    move.w d1,d2    ; For redrawing later
    lea (a4,d7.w),a0
    lea 1(a0),a1
.shiftRight:
    move.b -(a0),-(a1)
    dbra d1,.shiftRight    
    bsr.s .appendChar
    bsr.s .redrawRemainingLine
    bra.s .waitForChar
    
.appendChar:
    move.b d0,(a4,d6.w)
    jsr CONPUTC(a6)
    addq.w #1,d6
    addq.w #1,d7    
    rts

.lineBreak:
    lea LineBreakMsg(pc),a1
    jmp CONPUTS(a6)
    ; DONE
.eraseChar:
    tst.w d6
    beq.s .waitForChar
    subq.w #1,d6
    subq.w #1,d7    
    move.w d7,d0    
    sub.w d6,d0 
    move.w d0,d2 ; For redrawing later
    lea (a4,d6.w),a0
    lea 1(a0),a1
.shiftLeft:
    move.b (a1)+,(a0)+
.testShiftLeft:
    dbra d0,.shiftLeft
    
    move.b #8,d0
    jsr CONPUTC(a6)

    bsr .redrawRemainingLine   
    bra .waitForChar

.redrawRemainingLine:
    lea (a4,d6.w),a1
    jsr CONPUTS(a6)

    moveq #32,d0
    jsr CONPUTC(a6)

    move.w d2,d0
    addq.w #1,d0
    jmp CONCRSLEFT(a6)    

.readEscapeSequence:
    bsr PromptWaitKey
    cmp.b #'[',d0
    bne .waitForChar
    bsr PromptWaitKey
    cmp.b #'D',d0
    beq.s .moveLeft
    cmp.b #'C',d0
    beq.s .moveRight
    cmp.b #'H',d0
    beq.s .moveHome
    cmp.b #'F',d0
    beq.s .moveEnd
    bra .waitForChar
.moveLeft:
    tst.l d6
    beq .waitForChar
    moveq #1,d0
    jsr CONCRSLEFT(a6)
    subq.l #1,d6
    bra .waitForChar
.moveRight:
    cmp.w d7,d6
    beq .waitForChar
    addq.w #1,d6
    moveq #1,d0
    jsr CONCRSRIGHT(a6)
    bra .waitForChar
.moveHome:
    tst.w d6
    beq .waitForChar
    move.w d6,d0
    jsr CONCRSLEFT(a6)
    moveq #0,d6
    bra .waitForChar
.moveEnd:
    cmp.w d6,d7
    beq .waitForChar
    move.w d7,d0
    sub.w d6,d0
    jsr CONCRSRIGHT(a6)
    move.w d7,d6
    bra .waitForChar

PromptWaitKey:    
    jsr CONGETC(a6)
    tst.l d0
    bmi.s PromptWaitKey
    rts


PrintPrompt:
    lea MsgPrompt1(pc),a1
    jsr CONPUTS(a6)    
    jsr CONBOLD(a6)
    lea CurrentDir,a1
    jsr CONPUTS(a6)
    jsr CONNORMAL(a6)
    lea MsgPrompt2(pc),a1
    jmp CONPUTS(a6) 

CurrentDir:
    dc.b "/",0
    blk.b 10,0
MsgPrompt1:
    dc.b "[",0
MsgPrompt2:
    dc.b "]$ ",0    
    even
