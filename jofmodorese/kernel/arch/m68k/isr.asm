    incdir ../../../include
    include hardware.i

    xdef _isr_configure
    xref _timer_tick
    xref _current
    xref _sched_next
    xref _kpanic
    xref _panic_msg
    xref _panic_regs
    xref _panic_group0
    xref _panic_frame
    xref _panic_stack_top

;____________________________________________________________
;
; Exception stub, \1 = message, \2 = group0 or group12
;
; Interrupts are disabled first: the CPU does not raise the
; interrupt mask for exceptions, so the timer could otherwise
; switch task in the middle of the panic. The message is stored
; without using a register or the stack, so both still hold the
; values from when the exception occurred.
;____________________________________________________________
    macro EXC_STUB
    move.w #$2700,sr
    move.l #\1,_panic_msg
    bra \2
    endm

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
    ; First set all exception vectors with a generic handler.
    ; Vectors 25-31 (autovector interrupts) are left as they are,
    ; the BIOS UART interrupt is still in use.
    lea generic_error_isr,a0
    lea $08.w,a1              ; Vectors 2-24: CPU exceptions, spurious interrupt
    moveq #24-2,d1
.cpu_vectors:
    move.l a0,(a1)+
    dbra d1,.cpu_vectors
    lea $80.w,a1              ; Vectors 32-255: TRAP #n, reserved, user vectors
    move.w #255-32,d1
.trap_vectors:
    move.l a0,(a1)+
    dbra d1,.trap_vectors

    ; Now set specific vector handlers
    move.l #bus_error_isr,$08.w
    move.l #address_error_isr,$0c.w
    move.l #illegal_instruction_isr,$10.w
    move.l #division_by_zero_isr,$14.w
    move.l #chk_isr,$18.w
    move.l #trapv_isr,$1c.w
    move.l #privilege_violation_isr,$20.w
    move.l #trace_isr,$24.w
    move.l #line_a_isr,$28.w
    move.l #line_f_isr,$2c.w
    move.l #spurious_isr,$60.w

    lea dummy_isr(pc),a0
    move.l a0,IRQV_BLANK.w
    lea _timer_isr(pc),a0
    move.l a0,IRQV_TIMER.w
    move.b #TIMER_150HZ,OREG+TIMER_FREQ ; 150 Hz
    move.b #0,OREG+TIMER_ACT  ; Disable timer
    move.b #1,OREG+TIMER_ACT  ; Enable timer
    move.w #$2200,sr        ; mask level 2, accepts 3–7
    rts

;____________________________________________________________
;
; Exception handlers, all of them end in a kernel panic
;____________________________________________________________
bus_error_isr:            EXC_STUB msg_bus_error,group0
address_error_isr:        EXC_STUB msg_address_error,group0
illegal_instruction_isr:  EXC_STUB msg_illegal_instruction,group12
division_by_zero_isr:     EXC_STUB msg_div_zero,group12
chk_isr:                  EXC_STUB msg_chk,group12
trapv_isr:                EXC_STUB msg_trapv,group12
privilege_violation_isr:  EXC_STUB msg_privilege_violation,group12
trace_isr:                EXC_STUB msg_trace,group12
line_a_isr:               EXC_STUB msg_line_a,group12
line_f_isr:               EXC_STUB msg_line_f,group12
spurious_isr:             EXC_STUB msg_spurious,group12
generic_error_isr:        EXC_STUB msg_generic,group12

;____________________________________________________________
;
; Group 0 frame (bus and address error):
;
;   sp ->  special status word  (word)
;          fault address        (long)
;          instruction register (word)
;          SR                   (word)
;          PC                   (long)
;____________________________________________________________
group0:
    movem.l d0-d7/a0-a6,_panic_regs
    move.l sp,_panic_group0     ; Non-NULL: print the extra fields
    lea 8(sp),a0                ; a0 -> SR/PC, same as group 1/2
    bra.s common_handler

;____________________________________________________________
;
; Group 1/2 frame (all other exceptions):
;
;   sp ->  SR                   (word)
;          PC                   (long)
;____________________________________________________________
group12:
    movem.l d0-d7/a0-a6,_panic_regs
    clr.l _panic_group0
    move.l sp,a0                ; a0 -> SR/PC

;____________________________________________________________
;
; Leave the stack of the failing code (it may be what caused
; the exception) and print the panic on a stack of its own.
;____________________________________________________________
common_handler:
    move.l a0,_panic_frame
    move.l _panic_stack_top,sp
    jsr _kpanic                 ; Prints everything, never returns
.halt:
    bra.s .halt

dummy_isr:
    rte

msg_bus_error:
    dc.b "Bus error",0
msg_address_error:
    dc.b "Address error",0
msg_illegal_instruction:
    dc.b "Illegal instruction",0
msg_div_zero:
    dc.b "Division by zero",0
msg_chk:
    dc.b "CHK out of bounds",0
msg_trapv:
    dc.b "TRAPV overflow",0
msg_privilege_violation:
    dc.b "Privilege violation",0
msg_trace:
    dc.b "Trace",0
msg_line_a:
    dc.b "Line A instruction",0
msg_line_f:
    dc.b "Line F instruction",0
msg_spurious:
    dc.b "Spurious interrupt",0
msg_generic:
    dc.b "Unexpected exception",0
    even
