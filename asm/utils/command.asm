    incdir ..
    include hardware.i

UART_RTS_ENABLE_THRESHOLD EQU $c0
UART_RTS_DISABLE_THRESHOLD EQU $e0

START equ $10000
    org START

    bsr InstallISR

    move.l $4.w,a6
MainLoop:    
    lea MsgPrompt(pc),a1
    jsr -22(a6) ; Puts
.waitForChar:    
    bsr UartReadChar
    tst.l d0
    bmi.s .waitForChar
    cmp.b #13,d0
    beq.s .lineBreak
    jsr -16(a6) ; Putc
    bra.s .waitForChar
.lineBreak:
    bra MainLoop    
MsgPrompt:
    dc.b 13,10,"/$ ",0
    even
InstallISR:    
    move.w #$2700,sr          ; disable while configuring    
    lea UartISR,a0
    move.l a0,IRQV_UART
    clr.b UartRdPtr
    clr.b UartWrPtr
    lea UART_BASE,a5
    move.b #MCR_RTS,UART_MCR(a5)  ; Manual flow control, Assert RTS, allow sender to transmit
    move.b #IER_ERBI,UART_IER(a5)  ; Enable receiver interrupt
    move.w #$2400,sr        ; mask level 4, accepts 5–7 (UART)
    rts

UartISR:
    movem.l d0-d1/a0-a2,-(sp)
    lea UART_BASE,a0
    lea UartRdBuf,a1
    lea UartWrPtr,a2
.1:   
    moveq #0,d1    
    move.b (a2),d1
    move.b UART_RBR(a0),d0
    move.b d0,0(a1,d1.w)   ; UART_RdBuf[WrPtr] = byte    
    addq.b #1,d1
    move.b d1,(a2)
    sub.b UartRdPtr,d1
    cmp.b #UART_RTS_DISABLE_THRESHOLD,d1
    blo.s .2
    move.b UART_MCR(a0),d0
    andi.b #~MCR_RTS,d0     ; Clear bit 1
    move.b d0,UART_MCR(a0)  ; Deassert RTS = stop sender    
.2:
    btst.b #0,UART_LSR(a0)
    bne.s .1 ; More data
    
    movem.l (sp)+,d0-d1/a0-a2
    rte    

;____________________________________________________________
;
; Read character from serial
; Return D0 = -1 for no character, 0-255 for value
; Destroys D0-D1,A0-A1
;____________________________________________________________
UartReadChar:
    lea UART_BASE,a0
    lea UartRdBuf,a1

    move.b UartWrPtr,d0
    sub.b UartRdPtr,d0  
    cmp.b #UART_RTS_ENABLE_THRESHOLD,d0
    bhs.s .check_data     ; unread >= 224, keep RTS stopped

    ; unread < 224, assert RTS / allow sender
    move.b  UART_MCR(a0),d1
    ori.b   #MCR_RTS,d1
    move.b  d1,UART_MCR(a0)

.check_data:
    tst.b d0                  ; any data?
    bne.s .has_data
    moveq #-1,d0
    rts
.has_data:
    moveq #0,d1
    moveq #0,d0
    move.b UartRdPtr,d1
    move.b 0(a1,d1.w),d0       ; d0 = char

    addq.b #1,UartRdPtr     
    rts       
    
UartRdPtr EQU *
UartWrPtr EQU *+1
UartRdBuf EQU *+2
