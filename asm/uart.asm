    include hardware.i

FIFO_CONFIG equ (FCR_FIFO_ENABLE|FCR_FIFO_TRIGGER_14|FCR_RX_FIFO_RESET|FCR_TX_FIFO_RESET)
UART_RTS_ENABLE_THRESHOLD EQU $c0
UART_RTS_DISABLE_THRESHOLD EQU $e0

UARTInit:
    move.l #UartISR,IRQV_UART
    ;lea UartISR,a0
    ;move.l a0,IRQV_UART
    lea UartData,a0
    clr.b UartRdPtr(a0)
    clr.b UartWrPtr(a0)
    lea UART_BASE,a0
    move.b #$00,UART_IER(a0)            ; Disable interrupts
    move.b #$80,UART_LCR(a0)            ; Enable DLAB so DLL/DLM are accessible
    move.b #$01,UART_DLL(a0)            ; Set divisor 1 (115200 baud for 1.8432 MHz clock)
    move.b #$00,UART_DLM(a0)            ; -"-
    move.b #$03,UART_LCR(a0)            ; 8N1
    move.b #FIFO_CONFIG,UART_FCR(a0)    ; Enable FIFO, 14 bytes trigger
    ;move.b #MCR_AFE|MCR_DTR|MCR_RTS,UART_MCR(a5); Enable Auto-CTS/RTS and assert DTR     
    move.b #MCR_RTS,UART_MCR(a0)        ; Manual flow control, Assert RTS, allow sender to transmit
    move.b #IER_ERBI,UART_IER(a0)       ; Enable receiver interrupt

    move.b #$aa,UART_SCR(a0)            ; Test that UART connectivity works
    move.b $400.l,d0
    move.b UART_SCR(a0),d0
    cmp.b #$aa,d0
    bne.s .uartInitFail
    moveq #0,d0
    rts
.uartInitFail:
    moveq #1,d0    
    rts

UartISR:
    movem.l d0-d1/a0-a1,-(sp)
    lea UART_BASE,a0
    lea UartData,a1
.1:   
    moveq #0,d1    
    move.b UartWrPtr(a1),d1
    move.b UART_RBR(a0),d0
    move.b d0,UartRdBuf(a1,d1.w)   ; UART_RdBuf[WrPtr] = byte    
    addq.b #1,d1
    move.b d1,UartWrPtr(a1)
    sub.b UartRdPtr(a1),d1
    cmp.b #UART_RTS_DISABLE_THRESHOLD,d1
    blo.s .2
    move.b UART_MCR(a0),d0
    andi.b #~MCR_RTS,d0     ; Clear bit 1
    move.b d0,UART_MCR(a0)  ; Deassert RTS = stop sender    
.2:
    btst.b #0,UART_LSR(a0)
    bne.s .1 ; More data
    
    movem.l (sp)+,d0-d1/a0-a1
    rte    

;____________________________________________________________
;
; Write byte in D0
; A0 will be destroyed
;____________________________________________________________
UARTPutChar:
    lea UART_BASE,a0
PutChar_Wait:
    btst.b #5,UART_LSR(a0)   ; TX FIFO has room?
    beq.s PutChar_Wait
    move.b d0,UART_THR(a0)
    rts   

;____________________________________________________________
;
; Check if Uart has data available
;
; Z=0 data available
;____________________________________________________________
;UARTIsDataAvailable:
;    lea UART_BASE,a0
;    btst.b #0,UART_LSR(a0)
;    rts
;    lea UartData,a0
;    move.b UartWrPtr(a0),d0
;    sub.b UartRdPtr(a0),d0  
;    tst.b d0
;    rts   

;____________________________________________________________
;
; Read character in D0
; A0 will be destroyed
;____________________________________________________________
;UARTReadCharBlocking:
    ;lea UART_BASE,a0
;.1:
    ;btst.b #0,UART_LSR(a0)    ; RX FIFO has data?
    ;beq.s .1
    ;move.b UART_RBR(a0),d0
    ;rts
;.1:    
;    bsr UARTIsDataAvailable
;    beq.s .1
;    bra UARTReadChar

;____________________________________________________________
;
; Read character from UART
;
; Return D0 = -1 for no character, 0-255 for value
;
; Destroys D0-D1,A0-A1
;____________________________________________________________
UARTGetChar:
    lea UART_BASE,a0
    lea UartData,a1

    move.b UartWrPtr(a1),d0
    sub.b UartRdPtr(a1),d0  
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
    move.b UartRdPtr(a1),d1
    move.b UartRdBuf(a1,d1.w),d0       ; d0 = char

    addq.b #1,UartRdPtr(a1)     
    rts       
