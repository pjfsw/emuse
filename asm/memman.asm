;____________________________________________________________
;
; Memory manager
;____________________________________________________________

MEMMAN_BLOCK_SIZE equ 128
MEMMAN_BLOCK_MASK equ (MEMMAN_BLOCK_SIZE-1)
MEMMAN_CEILING    equ $400      ; Pointer containing amount of RAM

    rsreset
MEMMAN_NEXT    rs.l 1
MEMMAN_SIZE    rs.l 1
MEMMAN_PID     rs.l 1
MEMMAN_ATTR    rs.w 1
MEMMAN_RESRVD  rs.w 1
MEMMAN_SIZEOF  rs.b 0

MEMMAN_BASE equ $020000-MEMMAN_SIZEOF   ; Set lower later

;____________________________________________________________
;
; MemInit - Initialize memory manager, making all memory free
;____________________________________________________________
MemInit:
    lea MEMMAN_BASE,a0
    clr.l (a0)
    clr.l 4(a0)
    clr.l 8(a0)
    clr.w 12(a0)    
    rts

;____________________________________________________________
;
; MemAlloc - Allocate RAM
; D0 : Number of bytes (> 0) to allocate
; Returns pointer in D0 or 0 if not enough continous RAM
;
;  | Allocated |Next|->  | Allocated |Next|->  |Allocated |NULL|
;____________________________________________________________
MemAlloc:
    movem.l d2/a2,-(sp)
    bsr.s .memAllocInt
    movem.l (sp)+,d2/a2
    rts
.memAllocInt:
    add.l #MEMMAN_SIZEOF,d0
    add.l #MEMMAN_BLOCK_MASK,d0
    and.l #~(MEMMAN_BLOCK_MASK),d0
    lea MEMMAN_BASE,a0
    suba.l a1,a1
.findEmptySlot:
    tst.l MEMMAN_NEXT(a0)
    beq.s .foundEmptySlot
    move.l a0,a1 ; Save previous slot
    move.l MEMMAN_NEXT(a0),a0
    bra.s .findEmptySlot
.foundEmptySlot:    
    lea MEMMAN_CEILING,a2
    move.l a0,d1
    add.l MEMMAN_SIZE(a0),d1    ; Start of next block
    move.l d1,d2
    add.l d0,d2
    cmp.l (a2),d2
    bls.s .addSlot
    moveq #0,d0    
    rts
.addSlot:
    move.l d1,MEMMAN_NEXT(a0)    
    
    move.l d1,a0                ; Advance to our new block

    clr.l MEMMAN_NEXT(a0)
    move.l d0,MEMMAN_SIZE(a0)   ; Bytes reserved    
    clr.w MEMMAN_ATTR(a0)       ; No attrs for now
    clr.w MEMMAN_RESRVD(a0)     ; Just clear it for future compatibility
    lea MEMMAN_SIZEOF(a0),a0
    move.l a0,d0
    rts
    
;____________________________________________________________
;
; MemAvail - Get free RAM memory
; Returns number of bytes RAM free in D0
;
;____________________________________________________________
MemAvail:
    lea OSVARS_BASE,a0
    move.l OsRamSize(a0),d0     ; Max RAM amount
    lea MEMMAN_BASE,a0
.findLastAllocation:
    tst.l MEMMAN_NEXT(a0)
    beq.s .lastSlotFound
    move.l MEMMAN_NEXT(a0),a0
    move.l a0,d1
    cmp.l d0,d1
    bhs.s .noMemory
    bra.s .findLastAllocation
.lastSlotFound:
    move.l a0,d1
    add.l MEMMAN_SIZE(a0),d1
    sub.l d1,d0
    rts    
.noMemory:
    moveq #0,d0
    rts



