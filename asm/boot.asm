    include "hardware.i"

; boot.asm - Minimal 68k Header
    org $f00000

    dc.l RAM_SIZE     ; Initial Stack Pointer (Top of RAM)
    dc.l Start        ; Initial Program Counter (Where code begins)

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
Start:   
    move.b #OVR_OFF,OVR_REG
    bsr Blink           ; First blink means the CPU is executing code
    bsr Blink           ; Second blink means the OVR and stack is working
    bsr ConOpen
    bne.s BootTestNOK
    bsr Blink           ; Third blink means the UART is responding
BootTestNOK:
    bsr ConClr
    lea welcomeMsg(pc),a1
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

    ;bsr MMCStartTransfer
    ;bsr MMCSendByte
    ;bsr MMCReadByte
    ;bsr MMCEndTransfer
loop:
    bra loop
welcomeMsg:
    dc.b "JOFMODORE 68K BIOS V1.00",13,10
    dc.b "Copyright (C) 2026 Johan Fransson",13,10
    dc.b "All rights reserved.",13,10,0
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
