ROOTLIB_BASE equ $4
;____________________________________________________________
;
; CONGETC - Read character from console
; Input:
; Output: D0: Character 0-255 or -1 if no char available
;____________________________________________________________
CONGETC     equ -52

;____________________________________________________________
; Reserved 
;____________________________________________________________
ROOTRSVD0   equ -46

;____________________________________________________________
;
; CONPUTHEX8 - Write register as 16-bit hex value
; Input: D0: value to write
; Output:
;____________________________________________________________
CONPUTHEX8  equ -40

;____________________________________________________________
;
; CONPUTHEX16 - Write register as 16-bit hex value
; Input: D0: value to write
; Output:
;____________________________________________________________
CONPUTHEX16 equ -34

;____________________________________________________________
;
; CONPUTHEX32 - Write register as 32-bit hex value
; Input: D0: value to write
; Output:
;____________________________________________________________
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
