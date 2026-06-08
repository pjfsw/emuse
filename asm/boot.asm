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
    lea (welcomeMsg).l,a1
    bsr ConPuts
    bsr StartMonitor
    ;bsr MMCStartTransfer
    ;bsr MMCSendByte
    ;bsr MMCReadByte
    ;bsr MMCEndTransfer
loop:
    bra loop
welcomeMsg:
    dc.b "JOFMODORE SE",13,10
    dc.b "Copyright (C)2025 Johan Fransson",13,10,13,10,0
    even

    include monitor.asm
    include mmc.asm
    include console.asm
