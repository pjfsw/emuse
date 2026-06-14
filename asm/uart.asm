    include hardware.i

FIFO_CONFIG equ (FCR_FIFO_ENABLE|FCR_FIFO_TRIGGER_14|FCR_RX_FIFO_RESET|FCR_TX_FIFO_RESET)

UARTInit:
    lea UART_BASE,a5
    move.b #$00,UART_IER(a5)            ; Disable interrupts
    move.b #$80,UART_LCR(a5)            ; Enable DLAB so DLL/DLM are accessible
    move.b #$01,UART_DLL(a5)            ; Set divisor 1 (115200 baud for 1.8432 MHz clock)
    move.b #$00,UART_DLM(a5)            ; -"-
    move.b #$03,UART_LCR(a5)            ; 8N1
    move.b #FIFO_CONFIG,UART_FCR(a5)    ; Enable FIFO, 14 bytes trigger
    move.b #MCR_AFE|MCR_DTR|MCR_RTS,UART_MCR(a5); Enable Auto-CTS/RTS and assert DTR     
    ; move.b #IER_ERBI,UART_IER(a5)     ; Enable receiver interrupt
    move.b #$aa,UART_SCR(a5)            ; Test that UART connectivity works
    move.b $400.l,d0
    move.b UART_SCR(a5),d0
    cmp.b #$aa,d0
    bne.s UARTInit_Fail1
    moveq #0,d0
    rts
UARTInit_Fail1:
    moveq #1,d0    
    rts

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
; A0 will be destroyed
; Z=0 data available
;____________________________________________________________
UARTIsDataAvailable:
    lea UART_BASE,a0
    btst.b #0,UART_LSR(a0)
    rts

;____________________________________________________________
;
; Read character in D0
; A0 will be destroyed
;____________________________________________________________
UARTReadCharBlocking:
    lea UART_BASE,a0
.1:
    btst.b #0,UART_LSR(a0)    ; RX FIFO has data?
    beq.s .1
    move.b UART_RBR(a0),d0
    rts

    