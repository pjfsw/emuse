;____________________________________________________________
;
; BIOS root library wrappers
;
; Temporary console access through the BIOS until the kernel has
; its own drivers. The wrappers preserve all registers that vbcc
; expects to be preserved (d2-d7/a2-a6).
;____________________________________________________________
    include "rootlib.i"

    section "CODE",code

    xdef _bios_putc
    xdef _bios_puts
    xdef _bios_getc

; void bios_putc(REG("d0") int c)
_bios_putc:
    movem.l d2-d7/a2-a6,-(sp)
    move.l ROOTLIB_BASE,a6
    jsr CONPUTC(a6)
    movem.l (sp)+,d2-d7/a2-a6
    rts

; void bios_puts(REG("a1") const char *s)
_bios_puts:
    movem.l d2-d7/a2-a6,-(sp)
    move.l ROOTLIB_BASE,a6
    jsr CONPUTS(a6)
    movem.l (sp)+,d2-d7/a2-a6
    rts

; int bios_getc(void) - character 0-255, or -1 if none available
_bios_getc:
    movem.l d2-d7/a2-a6,-(sp)
    move.l ROOTLIB_BASE,a6
    jsr CONGETC(a6)
    movem.l (sp)+,d2-d7/a2-a6
    rts
