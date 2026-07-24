ROOTLIB_BASE equ $000004
ROOTLIB_VERSION equ -4
;____________________________________________________________
;
; LIBADD  - Install new or replace existing library
; Input: D0: 32-bit Library identifier
; 
;____________________________________________________________
LIBADD      equ -88

;____________________________________________________________
;
; LIBCLOSE - Close library 
; Input:  D0: 32-bit Library identifier
; Output: D0: 0=OK
;____________________________________________________________
LIBCLOSE    equ -82

;____________________________________________________________
;
; LIBOPEN - Open library 
; Input:  D0: 32-bit Library identifier
;         D1: Version
; Output: D0: Pointer to library base or -1 if unsuccessful
;____________________________________________________________
LIBOPEN     equ -76

;____________________________________________________________
;
; MEMAVAIL - Return the amount of free memory
; Input:  
; Output: D0: Free RAM in bytes
;____________________________________________________________
MEMAVAIL    equ -70

;____________________________________________________________
;
; MEMFREE - Free allocated memory 
; Input:  A0: Pointer to allocated region
; Output: D0: Character 0-255 or -1 if no char available
;____________________________________________________________
MEMFREE     equ -64

;____________________________________________________________
;
; MEMALLOC - Allocate memory 
; Input:  D0: Number of bytes to allocate
; Output: D0: pointer to allocated region or 0 if not enough
;            continous RAM
;____________________________________________________________
MEMALLOC    equ -58

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
; Input:  D0: value to write
; Output:
;____________________________________________________________
CONPUTHEX8  equ -40

;____________________________________________________________
;
; CONPUTHEX16 - Write register as 16-bit hex value
; Input:  D0: value to write
; Output:
;____________________________________________________________
CONPUTHEX16 equ -34

;____________________________________________________________
;
; CONPUTHEX32 - Write register as 32-bit hex value
; Input:  D0: value to write
; Output:
;____________________________________________________________
CONPUTHEX32 equ -28

;____________________________________________________________
;
; CONPUTS - Write null terminated string to console
; Input:  A1: Pointer to null terminated string
; Output:
;____________________________________________________________
CONPUTS     equ -22

;____________________________________________________________
;
; CONPUTC - Write character to console
; Input:  D0: Character 0-255 to write
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

