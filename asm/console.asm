ConOpen:
    bsr UARTInit    
    rts

;____________________________________________________________
;
; Clear screen
;____________________________________________________________
ConClr: 
    lea (ConClrMsg).l,a1
    bra ConPuts
ConClrMsg:  ; TODO this is UART specific, needs Terminal abstraction
    dc.b 27,"[2J",0
    even

;____________________________________________________________
;
; Write character (byte) in D0 to console
;____________________________________________________________
ConPutc:
    bra UARTPutChar

;____________________________________________________________
;
; Write a null-terminated string (A1) to console.
;____________________________________________________________
ConPuts:
    move.b (a1)+,d0
    beq.s .1
    bsr ConPutc
    bra.s ConPuts
.1:
    rts    

ConPutHexNibble:
    and.b #15,d0
    add.b #'0',d0
    cmp.b #':',d0
    bcs.s .1
    add.b #7,d0
.1:    
    bsr ConPutc
    rts

;____________________________________________________________
;
; Write a 32-bit hex value (D0) to console.
;____________________________________________________________
ConPutHex32:
    movem.l d2/d7,-(sp)
    move.l d0,d2   
    moveq #8,d7
.1:
    rol.l #4,d2
    move.b d2,d0
    bsr ConPutHexNibble
    subq #1,d7    
    bne.s .1

    movem.l (sp)+,d2/d7
    rts

;____________________________________________________________
;
; Write a 16-bit hex value (D0) to console.
;____________________________________________________________
ConPutHex16:
    movem.l d2/d7,-(sp)
    move.w d0,d2   
    moveq #4,d7
.1:
    rol.w #4,d2
    move.b d2,d0
    bsr ConPutHexNibble
    subq #1,d7    
    bne.s .1

    movem.l (sp)+,d2/d7
    rts

;____________________________________________________________
;
; Write a 8-bit hex value (D0) to console.
;____________________________________________________________
ConPutHex8:
    movem.l d2/d7,-(sp)
    move.w d0,d2   
    moveq #2,d7
.1:
    rol.b #4,d2
    move.b d2,d0
    bsr ConPutHexNibble
    subq #1,d7    
    bne.s .1

    movem.l (sp)+,d2/d7
    rts

;____________________________________________________________
;
; Read character from console
; Return character 0-255 in D0, or -1 if no char available
;____________________________________________________________
ConGetChar:
    bra UARTGetChar

    include uart.asm
