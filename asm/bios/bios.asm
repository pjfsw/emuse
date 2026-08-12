    incdir "../storage"
    incdir "../include"
    include "hardware.i"
    include "osvars.i"

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

    include "jt_dos.asm"
    include "jt_root.asm"   ; MUST BE IMMEDIATELY ABOVE "Start"
Start:
    move.b #OVR_OFF,OVR_REG       

    move.w #$2700,sr    ; disable interrupts while configuring    
    lea Start,a6
    move.l a6,EXEC_BASE

    bsr SetLedOn           ; Led ON means CPU is executing code
    Delay
    LedOff                 ; Led OFF means RAM/stack is working
    bsr ConOpen   
    move.w #$2400,sr    ; mask level 4, accepts 5–7 (UART)
    bsr DetectRam
    move.l DetectedRamSize,a7   ; Set top of RAM be stack pointer

    bsr ExceptionHandlerInit
    bsr LMInit

    bsr ConNormalText
    bsr ConClr
    lea welcomeMsg(pc),a1
    bsr ConPuts  
    
    move.l DetectedRamSize,d0
    lsr.l #4,d0
    lsr.l #6,d0
    lea OSVARS_BASE,a6
    lea OsScratchArea(a6),a0
    bsr GetU16DecimalString
    lea OsScratchArea(a6),a1
    bsr ConPuts
    lea detectedMsg(pc),a1
    bsr ConPuts
    bsr DOSInit

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
    cmp.b #13,d0
    beq StartBootLoader
    bra.s .waitChar

StartUploader:
    bsr Uploader
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

SetLedOn:
    LedOn
    rts

welcomeMsg:
    dc.b "JOFMODORE SE BIOS V1.00",13,10
    dc.b "Copyright (C) 2026 Johan Fransson",13,10
    dc.b "All rights reserved",13,10,0
LineBreakMsg:
    dc.b 13,10,0    
detectedMsg:
    dc.b "K RAM",13,10,0        
menuMsg:    
    dc.b 13,10
    dc.b "[U]pload hex data or [Enter] normal boot: "
    dc.b 0
    even

    include exceptions.asm
    include dos.asm
    include memman.asm
    include libman.asm
    include fileman.asm
    include exeloader.asm    
    include partman.asm
    include bootloader.asm
    include uploader.asm
    include console.asm
    include biosram.asm
    include decimal.asm