;____________________________________________________________
;
; Memory manager
;____________________________________________________________

MEMMAN_CEILING    equ $400      ; Pointer containing amount of RAM

    rsreset
MEMMAN_NEXT    rs.l 1
MEMMAN_SIZE    rs.l 1
MEMMAN_PID     rs.l 1
MEMMAN_ATTR    rs.w 1
MEMMAN_RESRVD  rs.w 1
MEMMAN_SIZEOF  rs.b 0

MEMMAN_BLOCK_SIZE equ MEMMAN_SIZEOF
MEMMAN_BLOCK_MASK equ (MEMMAN_BLOCK_SIZE-1)

;____________________________________________________________
;
; MemInit - Initialize memory manager, making all memory free
;           D0 - Earliest start of allocation space
;____________________________________________________________
MemInit:
    ; d0 last loaded byte address + 1 => add padding and even
    ; address
    add.l #2*MEMMAN_BLOCK_SIZE-1,d0
    and.l #~(MEMMAN_BLOCK_MASK),d0
    sub.l #MEMMAN_SIZEOF,d0
    
    lea OSVARS_BASE,a1    
    lea OsAllocatorStart(a1),a1    
    move.l d0,(a1)  ; Save allocator address to OsAllocatorStart
    move.l d0,a1    ; Use A1 for init memory below
;____________________________________________________________
;
; MemClearEntry - Clear current memory entry in A1
;____________________________________________________________
MemClearEntry:
    clr.l (a1)
    clr.l 4(a1)
    clr.l 8(a1)
    clr.l 12(a1)    
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
    movem.l d1-d2/a2,-(sp)
    bsr.s .memAllocInt
    movem.l (sp)+,d1-d2/a2
    rts
.memAllocInt:
    add.l #MEMMAN_SIZEOF,d0
    add.l #MEMMAN_BLOCK_MASK,d0
    and.l #~(MEMMAN_BLOCK_MASK),d0
    lea OSVARS_BASE,a0
    move.l OsAllocatorStart(a0),a0
    suba.l a1,a1    ; Previous ptr
    suba.l a2,a2    ; Best so far
    moveq #-1,d2    ; Best so far
    tst.l MEMMAN_SIZE(a0)
    bne.s .findNextSlot
    ; Initial slot, save entry directly without linking
    bra .storeEntry
.findNextSlot:
    move.l MEMMAN_NEXT(a0),d1   ; Next block
    beq.s .lastSlot
    move.l d1,a1
    bra.s .examineSlot
.lastSlot:
    move.l MEMMAN_CEILING,d1
    suba.l a1,a1
.examineSlot:    
    sub.l a0,d1
    sub.l MEMMAN_SIZE(a0),d1

    ; Does required allocation fit?
    cmp.l d0,d1
    blo.s .next

    ; Is this better than our previous best?
    cmp.l d2,d1  ; d1-d2
    bhs.s .next

    ; New best
    move.l d1,d2
    move.l a0,a2
.next:
    move.l a1,d1
    beq.s .noMoreSlots
    move.l a1,a0
    bra.s .findNextSlot
.noMoreSlots:
    move.l a2,d1    ; Found a slot?
    bne.s .slotFound
    moveq #0,d0     ; Not enough memory
    rts
.slotFound: 
    move.l a2,a0
    adda.l MEMMAN_SIZE(a2),a0   ; A0 = new entry pointer
    
    ; Link new entry
    move.l MEMMAN_NEXT(a2),MEMMAN_NEXT(a0)
    move.l a0,MEMMAN_NEXT(a2)              

    ; Fall through
        
; Size in D0
.storeEntry:
    move.l d0,MEMMAN_SIZE(a0)    
    clr.l MEMMAN_PID(a0)
    clr.w MEMMAN_ATTR(a0)
    clr.w MEMMAN_RESRVD(a0)
    lea MEMMAN_SIZEOF(a0),a0
    move.l a0,d0
    rts


    
;____________________________________________________________
;
; MemFree - Free allocated space pointed to by A0
; Returns D0=0 if successful, D0<0 if unsuccessful
;____________________________________________________________
MemFree:
    move.l a2,-(sp)
    bsr.s .memFreeInt
    movem.l (sp)+,a2
    rts
.memFreeInt:
    suba.l #MEMMAN_SIZEOF,a0
    lea OSVARS_BASE,a1
    move.l OsAllocatorStart(a1),a1
    suba.l a2,a2
.findSlot:
    move.l a1,d0
    beq.s .notFound

    cmpa.l a0,a1
    beq.s .foundSlot

    move.l a1,a2
    move.l MEMMAN_NEXT(a1),a1
    bra.s .findSlot

.foundSlot:
    move.l a2,d0
    beq.s .notFound  ; No memory before, special case!

    move.l MEMMAN_NEXT(a1),d0
    move.l d0,MEMMAN_NEXT(a2)
    ;move.l MEMMAN_SIZE(a1),d0
    bsr MemClearEntry
    moveq #0,d0
    rts
.notFound:
    moveq #-1,d0
    rts

;____________________________________________________________
;
; MemAvail - Get free RAM memory
; Returns number of bytes RAM free in D0
;____________________________________________________________
MemAvail:
    lea OSVARS_BASE,a0
    move.l OsRamSize(a0),d0     ; Max RAM amount
    lea OSVARS_BASE,a0
    move.l OsAllocatorStart(a0),a0
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

;____________________________________________________________
;
; MemTotal - Get total amount of RAM memory
; Returns total number of bytes RAM in D0
;____________________________________________________________
MemTotal:
    lea OSVARS_BASE,a0
    move.l OsRamSize(a0),d0
    rts
