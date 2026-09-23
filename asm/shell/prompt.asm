Prompt:
    bsr PrintPrompt
.waitForChar:    
    jsr CONGETC(a6)
    tst.l d0
    bmi.s .waitForChar    
    cmp.b #$7f,d0
    beq.s .eraseChar
    cmp.b #13,d0
    beq.s .lineBreak
    cmp.w #MAX_CMDLINE_LENGTH-1,d6
    bhs.s .waitForChar
    cmp.b #32,d0
    blo.s .waitForChar
    cmp.b #127,d0
    bhi.s .waitForChar
    move.b d0,(a4,d6.w)    
    addq.w #1,d6
    jsr CONPUTC(a6) ; CONPUTC
    bra.s .waitForChar
.lineBreak:
    lea LineBreakMsg(pc),a1
    jmp CONPUTS(a6)
    ; DONE
.eraseChar:
    tst.w d6
    beq.s .waitForChar
    subq.w #1,d6
    clr.b (a4,d6.w)
    move.b #8,d0
    jsr CONPUTC(a6)
    move.b #32,d0
    jsr CONPUTC(a6)
    move.b #8,d0
    jsr CONPUTC(a6)
    bra .waitForChar

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
