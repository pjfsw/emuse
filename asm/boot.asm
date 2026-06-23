    include "hardware.i"

; boot.asm - Minimal 68k Header
    org $f00000

    dc.l ALLOCATOR_BASE  ; Initial Stack Pointer
    dc.l Start           ; Initial Program Counter 

    macro Delay
    move.l #$14000,d0
loop\@:
    subq.l #1,d0
    bne.s loop\@
    endm

    macro LedOn
    move.b #3,OREG+SPI_CS
    endm

    macro LedOff
    move.b #0,OREG+SPI_CS
    endm

    org $f00400        ; Move past the vector table    

JT_ConGetblocking: ; -52
    jmp ConGetblocking
JT_ConIsDataAvailable: ; -46
    jmp ConIsDataAvailable
JT_ConPutHex8:  ; -40
    jmp ConPutHex8
JT_ConPutHex16: ; -34
    jmp ConPutHex16
JT_ConPutHex32: ; -28
    jmp ConPutHex32
JT_ConPuts: ; -22
    jmp ConPuts
JT_ConPutc: ; -16
    jmp ConPutc
JT_ConClr: ; -10
    jmp ConClr
    dc.l 1             ; Version
Start:
    move.b #OVR_OFF,OVR_REG    
    lea Start,a6
    move.l a6,EXEC_BASE
    
    bsr Blink           ; First blink means the CPU is executing code
    bsr Blink           ; Second blink means the OVR and stack is working
    bsr ConOpen    
    bne.s .1
    bsr Blink           ; Third blink means the UART is responding
    move.l d0,d0
.1:
    bsr DetectRam
    move.l detectedRamSize,a7   ; Set top of RAM be stack pointer

    bsr ConClr
    lea welcomeMsg(pc),a1
    bsr ConPuts  
    
    move.l detectedRamSize,d0
    bsr ConPutHex32
    
    lea detectedMsg(pc),a1
    bsr ConPuts

BootMenuLoop:
    inline
    lea menuMsg(pc),a1
    bsr ConPuts
.1
    bsr ConGetblocking
    cmp.b #'U',d0
    beq StartUploader
    cmp.b #'u',d0
    beq StartUploader
    cmp.b #'M',d0
    beq StartMonitor
    cmp.b #'m',d0
    beq StartMonitor
    cmp.b #13,d0
    beq StartBootLoader
    bra.s .1
    einline
StartUploader:
    bsr Uploader
    bra.s BootMenuLoop
StartMonitor:
    bsr Monitor
    bra.s BootMenuLoop
StartBootLoader:
    bsr BootLoader
    bra.s BootMenuLoop

DetectRam:
    lea ALLOCATOR_BASE,a0
    move.l #$12345678,d0
    move.l #$55aa55aa,d1
    move.l #RAM_LIMIT,d2
.1
    move.l d0,(a0)
    move.l d1,4(a0)
    cmp.l (a0),d0
    bne.s RamEnd
    cmp.l 4(a0),d1
    bne.s RamEnd
    cmp.l d2,d1
    beq.s RamEnd
    lea ALLOCATOR_BASE(a0),a0
    bra .1
RamEnd:
    move.l a0,detectedRamSize   
    rts


    ;bsr MMCStartTransfer
    ;bsr MMCSendByte
    ;bsr MMCReadByte
    ;bsr MMCEndTransfer
loop:
    bra loop
welcomeMsg:
    dc.b "JOFMODORE SE BIOS V1.00",13,10
    dc.b "Copyright (C) 2026 Johan Fransson",13,10
    dc.b "All rights reserved.",13,10
    dc.b "$",0
detectedMsg:
    dc.b " bytes RAM",13,10,0        
menuMsg:    
    dc.b 13,10
    dc.b "[U]pload S-records  [M]onitor or  [Enter] boot from disk: "
    dc.b 0
    even

Blink:
    LedOn
    Delay
    LedOff
    Delay
    Delay
    rts    

    include ramtest.asm
    include bootloader.asm
    include uploader.asm
    include monitor.asm
    include mmc.asm
    include console.asm
    include biosram.asm