    incdir "../include"    
    include "rootlib.i"

START equ $1000
    org START

    bsr Allocate240
    bsr Allocate240
    move.l d0,SavedSlot
    bsr Allocate240

    move.l SavedSlot,a0
    bsr Free

    bsr Allocate112
    bsr Allocate112

    move.l ROOTLIB_BASE,a6
    move.l #$1000,a0
    move.l #$800,d0
    bsr DumpMemory

    jmp *

Allocate240:
    move.l #240,d0
    bra Allocate

Allocate112:
    move.l #112,d0
    bra Allocate

    nop

Free:
    move.l ROOTLIB_BASE,a6
    jmp MEMFREE(a6)

Allocate:
    move.l d7,-(sp)
    move.l ROOTLIB_BASE,a6
    jsr MEMALLOC(a6)
    move.l d0,d7
    jsr CONPUTHEX32(a6)
    move.l d7,d0
    move.l (sp)+,d7
    rts
SavedSlot:
    dc.l 0

    incdir "../shell"
    include "dumpmemory.asm"