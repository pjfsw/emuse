    incdir ..
    include hardware.i

    macro Delay
    move.l #$34000,d0
loop\@:
    subq.l #1,d0
    bne.s loop\@
    endm

GETC equ -52


;START equ $10000
    ;org START
    code

    move.w sr,OldSR
    move.w #$2700,sr          ; disable while configuring    
    lea DummyISR,a0
    move.l a0,IRQV_BLANK
    lea TimerISR,a0
    move.l a0,IRQV_TIMER
    move.b #TIMER_150HZ,OREG+TIMER_FREQ ; 150 Hz
    move.b #0,OREG+TIMER_ACT  ; Disable timer
    move.b #1,OREG+TIMER_ACT  ; Enable timer
    move.w  #$2200,sr        ; mask level 2, accepts 3–7    

    move.l $4.w,a6
.flushChars:
    jsr GETC(a6)
    bpl.s .flushChars

.waitForChar:    
    jsr GETC(a6)
    tst.l d0
    bmi.s .waitForChar    

    move.w #$2700,sr          ; disable while configuring    
    move.b #0,OREG+TIMER_ACT  ; disable timer
    tst.b TIMERACK            ; Acknowledge interrupt    
    move.w OldSR,sr           ; restore interrupt
    moveq #0,d0
    rts

    
TimerISR:
    movem.l d0-d1/a0-a1,-(sp)
    lea BlinkCount,a0
    move.b (a0),d0
    subq.b #1,d0
    move.b d0,(a0)
    bne .1
    move.b #75,(a0)
    move.b BlinkState,d0    
    eor.b #3,d0
    move.b d0,BlinkState
    move.b d0,OREG+SPI_CS
.1:
    tst.b TIMERACK      ; Acknowledge interrupt
    movem.l (sp)+,d0-d1/a0-a1
    rte

DummyISR:
    rte    

    data
    
BlinkState:
    dc.b 0
BlinkCount:
    dc.b 75

    bss

OldSR:
    ds.w 1
