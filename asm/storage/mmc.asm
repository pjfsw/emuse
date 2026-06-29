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
MMC_CMD55    equ $77
MMC_ACMD41   equ $69
MMC_CMD17    equ $51
MMC_CMD24    equ $58

MMC_ERR_CMD0_FAILED    equ $1000
MMC_ERR_CMD8_FAILED    equ $2000
MMC_ERR_CMD55_FAILED   equ $3000
MMC_ERR_ACMD41_FAILED  equ $4000
MMC_ERR_CMD17_FAILED   equ $5000
MMC_ERR_CMD24_FAILED   equ $6000
MMC_ERR_WAIT_TIMEOUT   equ $0100
MMC_ERR_RESP_TIMEOUT   equ $0200
MMC_ERR_READ_TIMEOUT   equ $0300
MMC_ERR_ACMD41_TIMEOUT equ $0400
MMC_ERR_WRITE_ERROR    equ $0500
MMC_ERR_WRITE_TIMEOUT  equ $0600
MCC_ERR_NOT_IMPLEMENTED equ $ffff

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
    move.b #MMC_MOSI_BIT,(a5) ; Clear clock after last bit
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
    ;movem.l d0-d7/a0-a6,-(sp)
    ;jsr PUTHEX8(a4)
    ;movem.l (sp)+,d0-d7/a0-a6
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
    bsr.s MMCInitInt
    MmcDeselect
    RestoreRegisters
    rts
MMCInitInt:
    move.l $4.w,a4
    MmcDeselect    
    MmcMosiClockA5MisoA6
    bsr MMCDummyClocksInt
    moveq #9,d7
.cmd0Loop:
    bsr MMCSendCmd0
    cmp.b #1,d0
    bls.s .cmd0OK
    dbra d7,.cmd0Loop
    ; Init Failure
    or.w #MMC_ERR_CMD0_FAILED,d0
    rts
.cmd0OK:
    bsr MMCSendCmd8
    cmp.b #1,d0
    bls.s .cmd8OK
    ; Init Failure
    or.w #MMC_ERR_CMD8_FAILED,d0
    rts
.cmd8OK:
    bsr MMCReadByteInt
    bsr MMCReadByteInt
    bsr MMCReadByteInt
    bsr MMCReadByteInt

    move.w #$fff,d7
.cmd55Loop:
    bsr MMCSendCmd55
    cmp.b #1,d0
    bls.s .cmd55OK
    or.w #MMC_ERR_CMD55_FAILED,d0
    rts
.cmd55OK:    
    bsr MMCSendAcmd41
    cmp.b #1,d0
    beq.s .stillIdle
    blo.s .initDone
    or.w #MMC_ERR_ACMD41_FAILED,d0
    rts
.stillIdle:
    dbra d7,.cmd55Loop
    move.w #MMC_ERR_ACMD41_TIMEOUT,d0
    rts
.initDone:
    moveq #0,d0
    rts

NoArg:
    dc.l 0
MMCSendCmd0:
    lea Cmd0(pc),a0
    lea NoArg(pc),a1
    bra MMCSendCommandInt
Cmd0:
    dc.b MMC_CMD0,MMC_CMD0_CRC

MMCSendCmd8:
    lea Cmd8(pc),a0
    lea Cmd8Arg(pc),a1
    bra MMCSendCommandInt
Cmd8:
    dc.b MMC_CMD8,MMC_CMD8_CRC
Cmd8Arg:    
    dc.l $000001aa

MMCSendCmd55:
    lea Cmd55(pc),a0
    lea NoArg(pc),a1
    bra MMCSendCommandInt
Cmd55:
    dc.b MMC_CMD55,$FF

MMCSendAcmd41:
    lea Acmd41(pc),a0
    lea Acmd41Arg(pc),a1
    bra MMCSendCommandInt
Acmd41:
    dc.b MMC_ACMD41,$FF
Acmd41Arg:
    dc.l $40000000

;____________________________________________________________
;
; Read a sector
; Device number in D0 (currently ignored)
; Sector number in D1
; 512 byte buffer ptr in A0
; Returns D0 = 0: OK, D0 != 0: error code 
;____________________________________________________________
MMCReadSector:
    SaveRegisters
    bsr MMCReadSectorInt
    RestoreRegisters
    MmcDeselect
    rts
MMCReadSectorInt:
    move.l $4.w,a4
    MmcMosiClockA5MisoA6

    move.l a0,-(sp)
    lea Cmd17(pc),a0
    lea MmcCmdArg,a1
    move.l d1,(a1)
    bsr MMCSendCommandInt    
    move.l (sp)+,a0    
    tst.b d0
    beq.s .readCmdOk
    or.w #MMC_ERR_CMD17_FAILED,d0
    rts
.readCmdOk:
    move.w #255,d7
.waitReadToken:    
    bsr MMCReadByteInt    
    cmp.b #$fe,d0
    beq.s .readTokenOk
    dbra d7,.waitReadToken
    
    move.w #MMC_ERR_READ_TIMEOUT,d0    
    rts
.readTokenOk:
    ; READ BYTES TO BUFFER
    move.w #511,d7
.readNextByte:
    bsr MMCReadByteFastInt
    move.b d0,(a0)+
    dbra d7,.readNextByte
    bsr MMCReadByteFastInt
    bsr MMCReadByteFastInt
    bsr MMCReadByteFastInt
    moveq #0,d0
    rts
Cmd17:
    dc.b MMC_CMD17,$ff

;____________________________________________________________
;
; Write a sector
; Device number in D0 (currently ignored)
; Sector number in D1
; 512 byte buffer ptr in A0
; Returns D0 = 0: OK, D0 != 0: error code 
;____________________________________________________________
MMCWriteSector:
    SaveRegisters
    bsr .writeSectorInt
    RestoreRegisters
    MmcDeselect
    rts
.writeSectorInt:
    move.l $4.w,a4
    MmcMosiClockA5MisoA6
    move.l a0,-(sp)
    lea Cmd24(pc),a0
    lea MmcCmdArg,a1
    move.l d1,(a1)
    bsr MMCSendCommandInt    
    move.l (sp)+,a0    
    tst.b d0
    beq.s .writeCmdOk
    or.w #MMC_ERR_CMD24_FAILED,d0
    rts
.writeCmdOk:
    bsr MMCReadByteInt  ; send $FF but using read instead of write is slightly faster
    move.b #$fe,d0
    bsr MMCSendByteInt
    ; WRITE BYTES FROM BUFFER
    move.w #511,d7
.writeNextByte:
    move.b (a0)+,d0
    bsr MMCSendByteInt
    dbra d7,.writeNextByte
    bsr MMCReadByteInt  ; Send dummy CRC as $FF
    bsr MMCReadByteInt  ; -"-
    bsr MMCReadByteInt  ; Get write response
    and.b #$1f,d0
    move.b d0,d6        ; Save error code for later
    bsr MmcWaitBusy
    tst.l d0
    beq.s .waitOk
    move.w #MMC_ERR_WRITE_TIMEOUT,d0
    rts
.waitOk:
    moveq #0,d0
    cmp.b #$05,d6
    bne.s .writeError
    rts
.writeError:
    move.b d6,d0      
    or.w #MMC_ERR_WRITE_ERROR,d0
    rts
Cmd24:
    dc.b MMC_CMD24,$ff    


;____________________________________________________________
;
; A0 = Command and CRC
; A1 = Command arguments (4 bytes)
; Returns D0 = 0-1, OK > 1 = Failed
;____________________________________________________________
MMCSendCommandInt:   
    MmcSelect
    bsr MmcWaitBusy
    tst.l d0
    beq.s .waitOk
    rts
.waitOk:    
    move.b (a0)+,d0
    bsr MMCSendByteInt  ; Send command
    
    moveq #3,d7
.sendNextArgByte:
    move.b (a1)+,d0
    bsr MMCSendByteInt  ; Send argument
    dbra d7,.sendNextArgByte

    move.b (a0)+,d0
    bsr MMCSendByteInt  ; Send CRC

    ; Wait for response
    move.w #255,d7
.waitResponse:
    bsr MMCReadByteInt
    cmp.b #$ff,d0
    bne.s .commandComplete
    bsr.s ShortDelay
    dbra d7,.waitResponse
    move.w #MMC_ERR_RESP_TIMEOUT,d0
    rts
.commandComplete:
    and.l #$ff,d0
    rts ; Response in D0

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
