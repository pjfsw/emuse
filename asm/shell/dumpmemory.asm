; DumpMemory
;
; Input:
;   A0 = start address
;   D0 = number of bytes
;   A6 = ExecBase / console jump-table base
;
; Output format:
;
;   000208AA: 12 34 56 78 9A BC DE F0 01 02 03 04 05 06 07 08
;   000208BA: ...
;
; Preserves all registers except condition codes.

DumpMemory:
    movem.l d2-d7/a2-a5,-(sp)

    move.l  a0,a4                   ; Current memory pointer
    move.l  d0,d7                   ; Bytes remaining

.nextLine:
    tst.l   d7
    beq.s   .done

    ; Print address
    move.l  a4,d0
    jsr     CONPUTHEX32(a6)

    moveq   #':',d0
    jsr     CONPUTC(a6)

    moveq   #' ',d0
    jsr     CONPUTC(a6)

    moveq   #16,d6                  ; Maximum bytes on this line

.nextByte:
    tst.l   d7
    beq.s   .endLine

    moveq   #0,d0
    move.b  (a4)+,d0
    jsr     CONPUTHEX8(a6)

    moveq   #' ',d0
    jsr     CONPUTC(a6)

    subq.l  #1,d7
    subq.w  #1,d6
    bne.s   .nextByte

.endLine:
    moveq   #13,d0
    jsr     CONPUTC(a6)

    moveq   #10,d0
    jsr     CONPUTC(a6)

    bra.s   .nextLine

.done:
    movem.l (sp)+,d2-d7/a2-a5
    rts