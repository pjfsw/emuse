ROOTLIB_BASE equ $F00004
;____________________________________________________________
;
; CONGETC - Read character from console
; Input:
; Output: D0: Character 0-255 or -1 if no char available
;____________________________________________________________
CONGETC     equ -52
ROOTRSVD0   equ -46
CONPUTHEX8  equ -40
CONPUTHEX16 equ -34
CONPUTHEX32 equ -28

;____________________________________________________________
;
; CONPUTS - Write null terminated string to console
; Input: A1: Pointer to null terminated string
; Output:
;____________________________________________________________
CONPUTS     equ -22

;____________________________________________________________
;
; CONPUTC - Write character to console
; Input: D0: Character 0-255 to write
; Output:
;____________________________________________________________
CONPUTC     equ -16

;____________________________________________________________
;
; CONCLR - Clear console
; Input:
; Output:
;____________________________________________________________
CONCLR      equ -10

