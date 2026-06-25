    include "hardware.i"

MMC_CS_REG equ (OREG+SPI_CS)
MMC_MOSI_CLOCK equ (OREG+SPI_MOSI_CLK)
MMC_MISO equ IREG
MMC_CLK_BIT  equ (1<<0)
MMC_MOSI_BIT equ (1<<1)

MMC_CMD0     equ $40
MMC_CMD0_CRC equ $95
MMC_CMD8     equ $48
MMC_CMD8_CRC equ $87

MMC_ERR_CMD0_FAILED  equ $1000
MMC_ERR_CMD8_FAILED  equ $8000
MMC_ERR_WAIT_TIMEOUT equ $0100
MMC_ERR_RESP_TIMEOUT equ $0200

    macro MmcSelect
    move.b #SPI_CS_MMC,MMC_CS_REG ; Enable chip select
    endm

    macro MmcDeselect
    move.b #SPI_CS_OFF,MMC_CS_REG ; Disable chip select
    endm

    macro MmcMosiClockA5MisoA6
    lea MMC_MOSI_CLOCK,a5
    lea MMC_MISO,a6
    endm

    macro SaveRegisters
    movem.l d2-d7/a4-a6,-(sp)
    endm

    macro RestoreRegisters
    movem.l (sp)+,d2-d7/a4-a6
    endm

;____________________________________________________________
;
; MACRO MmcSendBit
; A5 - MOSI/CLOCK address
; D0 - the byte to send
; D1 - will be overwritten with clock/MOSI
;____________________________________________________________
    macro MmcSendBit

    moveq #0,d1            ; 4 cycles
    add.b d0,d0            ; 4 cycles
    bcc.s bit_is_zero\@    ; 8 (bit=1)/10 cycles (bit=0)
    moveq #MMC_MOSI_BIT,d1 ; 4 cycles (bit=1)
bit_is_zero\@:
    move.b d1,(a5)         ; 8 cycles
    addq.b #MMC_CLK_BIT,d1 ; 4 cycles, raise clock
    move.b d1,(a5)         ; 8 cycles
    endm                   ; 40 cycles (bit=1)/38 cycles (bit=0)

;____________________________________________________________
;
; MACRO MmcSendBit
; A5 - MOSI/CLOCK address
; A6 - MISO address
; D0 - the byte to send
; D1 - scratch register
; D2 - low clock + MOSI=1
; D3 - high clock + MOSI=1
;____________________________________________________________
    macro MmcReadBit
    move.b d3,(a5)      ; 8 cycles, MOSI=1, CLK=1
    move.b (a6),d1      ; 8 cycles, bit 7 is MISO
    add.b d1,d1         ; 4 cycles, bit 7 to C,X
    addx.b d0,d0        ; 4 cycles, shift X into d0
    move.b d2,(a5)      ; 8 cycles, MOSI=1, CLK=0
    endm                ; 32 cycles

    macro MmcLoadReadBitsToD2D3
    moveq #MMC_MOSI_BIT,d2
    moveq #MMC_MOSI_BIT|MMC_CLK_BIT,d3    
    endm

;____________________________________________________________
;
; Send byte in D0
; A5 Must contain MMC_MOSI_CLOCK
; D1 will be destroyed
;____________________________________________________________
MMCSendByteInt:
    MmcSendBit
    MmcSendBit
    MmcSendBit
    MmcSendBit
    MmcSendBit
    MmcSendBit
    MmcSendBit
    MmcSendBit          ; 304-320 cycles per byte
    move.b #0,(a5)
    rts

;____________________________________________________________
;
; Read byte and store into D0
; A5 must contain MMC_MOSI_CLOCK
; A6 must contain MMC_MISO
; D1-D3 will be destroyed
;____________________________________________________________
MMCReadByteInt:
    moveq #0,d0
    MmcLoadReadBitsToD2D3
MMCReadByteFastInt:
    MmcReadBit
    MmcReadBit
    MmcReadBit
    MmcReadBit
    MmcReadBit
    MmcReadBit
    MmcReadBit
    MmcReadBit          ; 256 cycles per byte
    movem.l d0-d7/a0-a6,-(sp)
    jsr PUTHEX8(a4)
    movem.l (sp)+,d0-d7/a0-a6
    rts


MMCDummyClocksInt:    
    ; Send dummy clocks to initialize MMC
    moveq #9,d7
.sendDummyClocks:
    bsr MMCReadByteInt
    dbra d7,.sendDummyClocks
    rts

MMCInit:
    SaveRegisters
    move.l $4.w,a4
    MmcDeselect    
    MmcMosiClockA5MisoA6
    bsr MMCDummyClocksInt
    moveq #9,d7
.cmd0Loop:
    bsr MMCSendCmd0
    tst.l d0
    beq.s .cmd0OK
    dbra d7,.cmd0Loop
    ; Init Failure
    or.w #MMC_ERR_CMD0_FAILED,d0
    bra .initDone
.cmd0OK:
    ;moveq #0,d0
    ;rts
    bsr MMCSendCmd8
    tst.l d0
    beq.s .cmd8OK
    ; Init Failure
    or.w #MMC_ERR_CMD8_FAILED,d0
    bra .initDone
.cmd8OK:
    moveq #0,d6
    bsr MMCReadByteInt
    or.b d0,d6
    bsr MMCReadByteInt
    asl.l #8,d6
    or.b d0,d6
    bsr MMCReadByteInt
    asl.l #8,d6
    or.b d0,d6
    bsr MMCReadByteInt
    asl.l #8,d6
    or.b d0,d6
    move.l d6,d0

    ;moveq #0,d0
    bra .initDone
    nop
.initDone:
    RestoreRegisters
    rts

MMCSendCmd0:
    lea Cmd0(pc),a0
    bra MMCSendCommandInt
Cmd0:
    dc.b MMC_CMD0,$00,$00,$00,$00,MMC_CMD0_CRC

MMCSendCmd8:
    lea Cmd8(pc),a0
    bra MMCSendCommandInt
Cmd8:
    dc.b MMC_CMD8,$00,$00,$01,$aa,MMC_CMD8_CRC
;____________________________________________________________
;
; A0 = Command structure (CMD, ARG, ARG, ARG, ARG, CRC)
; Returns D0 = 0: OK, D0 < 0 = Fail
;____________________________________________________________
MMCSendCommandInt:   
    MmcSelect
    bsr MmcWaitBusy
    tst.l d0
    beq.s .waitOk
    rts
.waitOk:
    moveq #5,d7
.sendNextByte:
    move.b (a0)+,d0
    bsr MMCSendByteInt
    dbra d7,.sendNextByte

    ; Wait for response
    move.w #255,d7
.waitResponse:
    bsr MMCReadByteInt
    cmp.b #$ff,d0
    bne.s .commandComplete
    bsr ShortDelay
    dbra d7,.waitResponse
    move.w #MMC_ERR_RESP_TIMEOUT,d0
    rts
.commandComplete:
    cmp.b #1,d0
    bhi.s .errorResponse
    moveq #0,d0
    rts
.errorResponse:      
    moveq #0,d0
    rts

ShortDelay:
    move.l d7,-(sp)
    move.w #255,d7
.loop:
    dbra d7,.loop
    move.l (sp)+,d7
    rts

MmcWaitBusy:
    move.w #$7fff,d7
.wait:    
    bsr MMCReadByteInt
    cmp.b #$ff,d0
    beq.s .done
    dbra d7,.wait
    move.w #MMC_ERR_WAIT_TIMEOUT,d0
    rts
.done:
    moveq #0,d0
    rts

