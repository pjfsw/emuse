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

JT_ConGetChar: ; -52
    jmp ConGetChar
JT_ConReserved0 ; -46
    blk.b 6,0
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
    move.w #$2700,sr    ; disable interrupts while configuring    
    move.w #$aaaa,d0
    ori.w #$5555,d0
    lea Start,a6
    move.l a6,EXEC_BASE
    
    bsr Blink           ; First blink means the CPU is executing code
    bsr Blink           ; Second blink means the OVR and stack is working
    bsr ConOpen    
    move.w #$2400,sr    ; mask level 4, accepts 5–7 (UART)
    bne.s .1
    bsr Blink           ; Third blink means the UART is responding
    move.l d0,d0
.1:
    bsr DetectRam
    move.l DetectedRamSize,a7   ; Set top of RAM be stack pointer

    bsr ConClr
    lea welcomeMsg(pc),a1
    bsr ConPuts  
    
    move.l DetectedRamSize,d0
    bsr ConPutHex32
    
    lea detectedMsg(pc),a1
    bsr ConPuts

BootMenuLoop:
    lea menuMsg(pc),a1
    bsr ConPuts
.waitChar
    bsr ConGetChar
    tst.l d0
    bmi .waitChar
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
    bra.s .waitChar

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
    move.l a0,DetectedRamSize   
    rts

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

    include bootloader.asm
    include uploader.asm
    include monitor.asm
    include console.asm
    include biosram.asm