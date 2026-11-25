    incdir ../../../include
    include hardware.i

    xdef _isr_configure
    xref _timer_tick
    xref _current
    xref _sched_next

;____________________________________________________________
;
; Timer interrupt, switches task when the time slice is used up.
;
; The CPU has pushed SR and PC, saving all registers on top of
; that gives the same frame as task_yield (see switch.asm).
;____________________________________________________________
_timer_isr:
    movem.l d0-d7/a0-a6,-(sp)
    tst.b TIMERACK
    jsr _timer_tick             ; D0 != 0: switch task
    tst.l d0
    beq.s .resume
    move.l _current,a0
    move.l sp,(a0)              ; current->sp = sp
    jsr _sched_next             ; Updates current
    move.l _current,a0
    move.l (a0),sp              ; sp = current->sp
.resume:
    movem.l (sp)+,d0-d7/a0-a6
    rte

_isr_configure:
    move.w #$2700,sr          ; disable while configuring    
    lea _dummy_isr(pc),a0
    move.l a0,IRQV_BLANK.w
    lea _timer_isr(pc),a0
    move.l a0,IRQV_TIMER.w
    move.b #TIMER_150HZ,OREG+TIMER_FREQ ; 150 Hz
    move.b #0,OREG+TIMER_ACT  ; Disable timer
    move.b #1,OREG+TIMER_ACT  ; Enable timer
    move.w #$2200,sr        ; mask level 2, accepts 3–7  
    rts  

_dummy_isr:
    rte
