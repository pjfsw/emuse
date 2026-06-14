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

ConIsDataAvailable:
    bra UARTIsDataAvailable
;____________________________________________________________
;
; Read character from console and store (byte) in D0
;____________________________________________________________
ConGetblocking:
    bra UARTReadCharBlocking

    include uart.asm

    rsset SYSTEM_BSS_BASE
__bss_start equ __RS
; label: rs.l 1
__bss_end equ __RS    