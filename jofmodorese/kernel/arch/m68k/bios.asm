;____________________________________________________________
;
; BIOS root library wrappers
;
; Temporary console access through the BIOS until the kernel has
; its own drivers. The wrappers preserve all registers that vbcc
; expects to be preserved (d2-d7/a2-a6).
;
; The BIOS is not re-entrant, so each call runs with the interrupt
; mask raised to at least 3: the timer (level 3) can not switch
; tasks during the call, while the UART (level 5) still works.
;____________________________________________________________
    include "rootlib.i"

    section "CODE",code

    xdef _bios_putc
    xdef _bios_puts
    xdef _bios_getc
    xdef _bios_put_hex32

;____________________________________________________________
;
; Call BIOS function \1 with the interrupt mask raised to at
; least 3. D0/A1 (arguments) are not touched before the call,
; D0 (result) is not touched after it.
;____________________________________________________________
    macro BiosCall
    movem.l d2-d7/a2-a6,-(sp)
    move.w sr,d1
    move.w d1,-(sp)             ; Save SR
    and.w #$0700,d1
    cmp.w #$0300,d1
    bhs.s .masked\@             ; Never lower the mask
    move.w #$2300,sr
.masked\@:
    move.l ROOTLIB_BASE,a6
    jsr \1(a6)
    move.w (sp)+,sr             ; Restore SR
    movem.l (sp)+,d2-d7/a2-a6
    rts
    endm

; void bios_putc(REG("d0") int c)
_bios_putc:
    BiosCall CONPUTC

; void bios_puts(REG("a1") const char *s)
_bios_puts:
    BiosCall CONPUTS

; int bios_getc(void) - character 0-255, or -1 if none available
_bios_getc:
    BiosCall CONGETC

; void bios_put_hex32(REG("d0") uint32_t value)
_bios_put_hex32:
    BiosCall CONPUTHEX32
