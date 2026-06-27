    incdir ..
    include hardware.i

UART_RTS_ENABLE_THRESHOLD EQU $c0
UART_RTS_DISABLE_THRESHOLD EQU $e0
PUTHEX8 equ -40
PUTHEX16 equ -34
PUTHEX32 equ -28
PUTS equ -22
PUTC equ -16

START equ $10000
    org START

    move.l $4.w,a6
    bsr InstallISR
    bsr RegisterStorageDevices
    bsr ReadMbr
.skipMmc:
MainLoop:    
    lea MsgPrompt(pc),a1
    jsr PUTS(a6) ; Puts
.waitForChar:    
    bsr UartReadChar
    tst.l d0
    bmi.s .waitForChar
    cmp.b #13,d0
    beq.s .lineBreak
    jsr PUTC(a6) ; Putc
    bra.s .waitForChar
.lineBreak:
    bra MainLoop    
MsgPrompt:
    dc.b 13,10,"[/]$ ",0
    even

RegisterStorageDevices:
    bsr SDInit
    bra RegisterMmc

RegisterMmc:
    lea MmcInitMsg(pc),a1
    jsr PUTS(a6)
    
    bsr MMCInit  
    move.w d0,MmcStatus
    tst.w d0
    bne.s .mmcFailed
    lea MmcStorageDevice,a0
    bsr SDRegisterDevice          
    jsr PUTHEX32(a6)

.mmcFailed:    
    bra PrintReturnCode
    rts

PrintReturnCode:
    move.l d7,-(sp)
    move.w MmcStatus,d7
    lea MmcErrorMsg(pc),a1
    jsr PUTS(a6)
    move.w d7,d0
    jsr PUTHEX16(a6)
    lea LineBreakMsg(pc),a1
    jsr PUTS(a6)
    move.l (sp)+,d7
    rts

ReadMbr:
    move.l #'mmc0',d0
    bsr SDFindDevice
    tst.l d0
    bpl.s .deviceFound 
    lea DeviceNotFoundMsg(pc),a1
    jmp PUTS(a6)
    rts
.deviceFound:    
    moveq #0,d1 ; Sector
    lea SectorBuffer,a0
    bsr SDReadSector
    tst.w d0
    bne PrintReturnCode
    bra PrintSector

PrintSector:
    movem.l d5-d7/a2,-(sp)    
    bsr .printSector
    movem.l (sp)+,d5-d7/a2
    rts
.printSector:
    lea SectorBuffer,a2
    moveq #31,d7
.nextRow:
    moveq #15,d6
    moveq #0,d5
.nextHexCol:
    move.b (a2,d5.w),d0
    jsr PUTHEX8(a6)
    move.b #' ',d0
    jsr PUTC(a6)
    addq.w #1,d5
    dbra d6,.nextHexCol

    moveq #15,d6
    moveq #0,d5
.nextAsciiCol:        
    move.b (a2,d5.w),d0
    cmp.b #31,d0
    bhi.s .lowerBoundOk
    move.b #'.',d0
    bra.s .asciiOk
.lowerBoundOk:
    cmp.b #128,d0
    blo.s .asciiOk
    move.b #'.',d0
.asciiOk:
    jsr PUTC(a6)
    addq.w #1,d5
    dbra d6,.nextAsciiCol

    lea 16(a2),a2
    lea LineBreakMsg(pc),a1
    jsr PUTS(a6)
    dbra d7,.nextRow
    rts


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

DeviceNotFoundMsg:
    dc.b "Device not found",13,10,0
MmcInitMsg:
    dc.b "MMC Initialization",13,10,0
MmcErrorMsg:
    dc.b "MMC Response code ",0
LineBreakMsg:
    dc.b 13,10,0    
    even

MmcStorageDevice:
    dc.l "mmc0"
    dc.l MMCReadSector
    dc.l MMCWriteSector
    blk.l 5,0

    include mmc.asm
    include storagedevice.asm

SectorBuffer EQU *
SDDeviceList EQU SectorBuffer+512 
MmcStatus    EQU SDDeviceList+512
MmcCmdArg    EQU MmcStatus+4
UartRdPtr    EQU MmcCmdArg+4
UartWrPtr    EQU UartRdPtr+1
UartRdBuf    EQU UartWrPtr+1
    