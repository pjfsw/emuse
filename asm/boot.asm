    include "hardware.i"

; boot.asm - Minimal 68k Header
    org $f00000

    dc.l RAM_SIZE     ; Initial Stack Pointer (Top of RAM)
    dc.l Start        ; Initial Program Counter (Where code begins)

    org $f00400        ; Move past the vector table
Start:   
    move.b #OVR_OFF,OVR_REG
    ;lea $40000,a0
    ;move.b #1,(a0)
    ;btst.b #0,(a0)
    ;btst.b #1,(a0)
    bsr ConOpen    
    bsr ConClr
    lea welcomeMsg(pc),a1
    bsr ConPuts
    
    inline
.1
    bsr ConGetblocking
    cmp.b #'U',d0
    beq Uploader
    cmp.b #'u',d0
    beq Uploader
    cmp.b #'M',d0
    beq Monitor
    cmp.b #'m',d0
    beq Monitor
    cmp.b #13,d0
    beq BootLoader
    bra .1
    einline

    ;bsr MMCStartTransfer
    ;bsr MMCSendByte
    ;bsr MMCReadByte
    ;bsr MMCEndTransfer
loop:
    bra loop
welcomeMsg:
    dc.b "JOFMODORE 68K BIOS V1.00",13,10
    dc.b "Copyright (C) 2026 Johan Fransson",13,10
    dc.b "All rights reserved.",13,10,13,10
    dc.b "[U]pload S-records  [M]onitor or  [Enter] boot from disk: ",0
    even

    include bootloader.asm
    include uploader.asm
    include monitor.asm
    include mmc.asm
    include console.asm
