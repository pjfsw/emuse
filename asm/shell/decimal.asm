;____________________________________________________________
;
; Convert 16-bit word in D0 to decimal ASCII
; A0: Pointer where to store target string (6 bytes needed); 
;____________________________________________________________
PrintU16Decimal:
    move.l d2,-(sp)
    lea 5(a0),a1
    clr.b (a1)
    moveq #10,d1
    and.l #$ffff,d0
.loop:
    divu d1,d0             ; d0 = quotient:remainder
    move.l d0,d2
    swap d2                ; d3.w = remainder
    add.b #'0',d2
    move.b d2,-(a1)

    and.l #$0000ffff,d0     ; keep quotient
    bne.s .loop

.copy:
    move.b (a1)+,(a0)+
    bne.s .copy
    move.l (sp)+,d2
    rts

;____________________________________________________________
;
; Convert 32-bit word in D0 to decimal ASCII
; A0: Pointer where to store target string (11 bytes needed); 
; Return: A0 will contain the offset of the NULL termination
;____________________________________________________________
PrintU32Decimal:
    movem.l d2-d3,-(sp)
    lea     10(a0),a1
    clr.b   (a1)
.loop:
    bsr  DivU32By10        ; d0 = quotient, d1 = remainder 0..9
    add.b  #'0',d1
    move.b d1,-(a1)
    tst.l d0
    bne.s .loop
    
.copy:
    move.b  (a1)+,(a0)+
    bne.s   .copy
    movem.l (sp)+,d2-d3
    rts

DivU32By10:
    move.l  d0,d2             ; dividend shift register
    moveq   #0,d0             ; quotient
    moveq   #0,d1             ; remainder
    moveq   #31,d3
.loop:
    add.l   d2,d2             ; next dividend bit -> X/C
    addx.l  d1,d1             ; remainder = remainder*2 + bit

    cmp.l   #10,d1
    blo.s   .no_sub
    sub.l   #10,d1
    addq.l  #1,d0             ; set quotient bit
.no_sub:
    dbra    d3,.shiftq
    rts

.shiftq:
    add.l   d0,d0             ; shift quotient left for next bit
    bra.s   .loop