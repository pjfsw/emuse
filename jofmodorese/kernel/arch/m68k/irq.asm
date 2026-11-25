;____________________________________________________________
;
; Interrupt mask helpers for critical sections
;
;   uint16_t sr = irq_save();
;   ... code that must not be interrupted ...
;   irq_restore(sr);
;____________________________________________________________
    section "CODE",code

    xdef _irq_save
    xdef _irq_restore

;____________________________________________________________
;
; uint16_t irq_save(void)
;
; Disable all interrupts, returns the previous SR.
;____________________________________________________________
_irq_save:
    moveq #0,d0
    move.w sr,d0
    move.w #$2700,sr
    rts

;____________________________________________________________
;
; void irq_restore(REG("d0") uint16_t sr)
;
; Restore SR as returned by irq_save.
;____________________________________________________________
_irq_restore:
    move.w d0,sr
    rts
