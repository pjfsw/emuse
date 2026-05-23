MMC_CS_REG equ $e00000
MMC_MOSI_CLOCK equ $e00002
MMC_MISO equ $e00004
MMC_CLK_BIT equ (1<<0)
MMC_MOSI_BIT equ (1<<1)

;____________________________________________________________
;
; MACRO MmcSendBit
; A0 - MOSI/CLOCK address
; D0 - the byte to send
; D1 - will be overwritten with clock/MOSI
;____________________________________________________________
    macro MmcSendBit

    moveq #0,d1            ; 4 cycles
    add.b d0,d0            ; 4 cycles
    bcc.s bit_is_zero\@    ; 8 (bit=1)/10 cycles (bit=0)
    moveq #MMC_MOSI_BIT,d1 ; 4 cycles (bit=1)
bit_is_zero\@:
    move.b d1,(a0)         ; 8 cycles
    addq.b #MMC_CLK_BIT,d1 ; 4 cycles, raise clock
    move.b d1,(a0)         ; 8 cycles
    endm                   ; 40 cycles (bit=1)/38 cycles (bit=0)

;____________________________________________________________
;
; MACRO MmcSendBit
; A0 - MOSI/CLOCK address
; A1 - MISO address
; D0 - the byte to send
; D1 - scratch register
; D2 - low clock + MOSI=1
; D3 - high clock + MOSI=1
;____________________________________________________________
    macro MmcReadBit
    move.b (a1),d1      ; 8 cycles, bit 7 is MISO
    add.b d1,d1         ; 4 cycles, bit 7 to carry
    addx.b d0,d0        ; 4 cycles, shift carry into d0
    move.b d2,(a0)      ; 8 cycles, MOSI=1, CLK=1
    move.b d3,(a0)      ; 8 cycles, MOSI=1, CLK=0
    endm                ; 32 cycles


;____________________________________________________________
;
; Enable chip select on MMC
; Populate A0 with MOSI/CLK and A1 with MISO
;____________________________________________________________
MMCStartTransfer:
    lea MMC_CS_REG,a0       ; Chip select
    move.b #0,(a0)          ; Enable chip select
    lea MMC_MOSI_CLOCK,a0  
    lea MMC_MISO,a1
    rts

;____________________________________________________________
;
; Disable chip select on MMC
; Populate A0 with MOSI/CLK and A1 with MISO
;____________________________________________________________
MMCEndTransfer:
    lea MMC_CS_REG,a0       ; Chip select
    move.b #1,(a0)          ; Disable chip select
    rts


;____________________________________________________________
;
; Send byte in D0
; A0 Must contain MMC_MOSI_CLOCK
; D1 will be destroyed
;____________________________________________________________
MMCSendByte:
    MmcSendBit
    MmcSendBit
    MmcSendBit
    MmcSendBit
    MmcSendBit
    MmcSendBit
    MmcSendBit
    MmcSendBit          ; 304-320 cycles per byte
    rts

;____________________________________________________________
;
; Read byte and store into D0
; A0 must contain MMC_MOSI_CLOCK
; A1 must contain MMC_MISO
; D1-D3 will be destroyed
;____________________________________________________________
mmcReadByte:
    MmcReadBit
    MmcReadBit
    MmcReadBit
    MmcReadBit
    MmcReadBit
    MmcReadBit
    MmcReadBit
    MmcReadBit          ; 256 cycles per byte
    rts
