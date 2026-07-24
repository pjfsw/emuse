;SP+00  SR                (word)
;SP+02  PC                (long)
;SP+06  IR                (word)
;SP+08  Fault Address     (long)
;SP+0C  Special Status    (word)
ExceptionAddressError:
    move.w #$2700,sr    ; Disable interrupts
    move.l 10(sp),d2 ; PC
    move.l 2(sp),d3 ; Fault address
    move.w 6(sp),d4 ; IR

    move.l ROOTLIB_BASE,a6

    lea .msg(pc),a1
    jsr CONPUTS(a6)

    move.l d2,d0
    jsr CONPUTHEX32(a6)

    lea .addrMsg(pc),a1
    jsr CONPUTS(a6)
    move.l d3,d0
    jsr CONPUTHEX32(a6)

    lea .irMsg(pc),a1
    jsr CONPUTS(a6)
    moveq #0,d0
    move.w d4,d0
    jsr CONPUTHEX16(a6)

    lea ExceptionEndMsg(pc),a1
    jsr CONPUTS(a6)

.loop:
    bra.s .loop

.msg:
    dc.b 13,10
    dc.b "*** ADDRESS ERROR ***  PC: $",0
.addrMsg:
    dc.b " Fault Address: $",0
.irMsg:
    dc.b " IR: $",0
    even

ExceptionIllegalInstruction:
    move.w  #$2700,sr

    ; Capture before console routines use the stack
    move.l 2(sp),d5           ; PC associated with exception
    move.w (sp),d6            ; Previous SR
    
    move.l ROOTLIB_BASE,a6

    lea .msg(pc),a1
    jsr CONPUTS(a6)

    move.l d5,d0
    jsr CONPUTHEX32(a6)

    lea .srMsg(pc),a1
    jsr CONPUTS(a6)

    move.w d6,d0
    jsr CONPUTHEX16(a6)

    lea ExceptionEndMsg(pc),a1
    jsr CONPUTS(a6)

.loop:
    bra.s  .loop

.msg:
    dc.b 13,10
    dc.b "*** ILLEGAL INSTRUCTION ***  PC: $",0
.srMsg:
    dc.b " SR: $",0
    even
ExceptionEndMsg:
    dc.b 13,10,"Execution terminated.",13,10,0

