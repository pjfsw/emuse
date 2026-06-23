    incdir ..
    include hardware.i

    macro Delay
    move.l #$34000,d0
loop\@:
    subq.l #1,d0
    bne.s loop\@
    endm


START equ $10000
    org START
    move.w  #$2700,sr          ; disable while configuring    
    lea DummyISR,a0
    move.l a0,IRQV_BLANK
    move.l a0,IRQV_UART
    lea TimerISR,a0
    move.l a0,IRQV_TIMER
    move.b #TIMER_150HZ,OREG+TIMER_FREQ ; 150 Hz
    move.b #0,OREG+TIMER_ACT  ; Disable timer
    move.b #1,OREG+TIMER_ACT  ; Enable timer
    move.w  #$2200,sr        ; mask level 2, accepts 3–7    
    bra *

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

;    move.l $4,a6
    ;lea OREG+SPI_CS,a5
    ;moveq #0,d2
;loop:
    ;addq.w #1,d2
    ;move.w d2,d0
    ;jsr -34(a6)  ; conputhex16
    ;move.b #3,(a5)
    ;Delay
    ;move.b #0,(a5)
    ;Delay
    ;bra loop
;Msg:
    ;dc.b "!",13,10,0
    ;even

BlinkState:
    dc.b 0
BlinkCount:
    dc.b 75
