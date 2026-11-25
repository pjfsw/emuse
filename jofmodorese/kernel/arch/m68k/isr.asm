    incdir ../../../include
    include hardware.i

    xdef _isr_configure
    xref _timer_tick

_timer_isr:
    movem.l d0-d1/a0-a1,-(sp)
    tst.b TIMERACK
    jsr _timer_tick
    movem.l (sp)+,d0-d1/a0-a1
    rte

_isr_configure:
    move.w #$2700,sr          ; disable while configuring    
    lea _dummy_isr(pc),a0
    move.l a0,IRQV_BLANK
    lea _timer_isr(pc),a0
    move.l a0,IRQV_TIMER
    move.b #TIMER_150HZ,OREG+TIMER_FREQ ; 150 Hz
    move.b #0,OREG+TIMER_ACT  ; Disable timer
    move.b #1,OREG+TIMER_ACT  ; Enable timer
    move.w #$2200,sr        ; mask level 2, accepts 3–7  
    rts  

_dummy_isr:
    rte
