;____________________________________________________________
;
; Task context switch
;
; The context of a task is saved on its own stack, in the same
; layout as an interrupt would leave it, so that a later timer
; interrupt can switch tasks using the same frame:
;
;   sp ->  d0-d7/a0-a6   (15 longwords)
;          SR            (word)
;          PC            (longword)
;
; task_t.sp (offset 0) holds the saved stack pointer.
;____________________________________________________________
    section "CODE",code

    xdef _task_yield
    xdef _task_run_first
    xref _current
    xref _sched_next

;____________________________________________________________
;
; void task_yield(void)
;
; Save the context of the current task, let the scheduler pick
; the next task and continue running that one.
;____________________________________________________________
_task_yield:
    move.w sr,-(sp)             ; PC (from jsr) + SR = exception frame
    move.w #$2700,sr            ; No interrupts while switching
    movem.l d0-d7/a0-a6,-(sp)
    move.l _current,a0
    move.l sp,(a0)              ; current->sp = sp
    jsr _sched_next             ; Updates current
    ; Fall through

;____________________________________________________________
;
; void task_run_first(void)
;
; Restore the context of the current task and run it.
;____________________________________________________________
_task_run_first:
    move.l _current,a0
    move.l (a0),sp              ; sp = current->sp
    movem.l (sp)+,d0-d7/a0-a6
    rte                         ; Restores SR and PC
