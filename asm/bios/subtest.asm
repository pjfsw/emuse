
TestSubSubx:
    movem.l d0-d3/d7,-(sp)
    bsr.s .test
; ==========================================================
; SUCCESS
; ==========================================================
    movem.l (sp)+,d0-d3/d7
    rts


.test:
; ==========================================================
; TEST 1: SUB.B - no borrow, upper bits preserved
; $44 - $0A = $3A
; ==========================================================
    moveq   #1,d7
    move.l  #$0000db44,d0
    moveq   #10,d1
    sub.b   d1,d0
    bcs     .fail
    bvs     .fail
    bmi     .fail
    beq     .fail
    cmpi.l  #$0000db3a,d0
    bne     .fail

; ==========================================================
; TEST 2: SUB.B - borrow
; $00 - $01 = $FF
; C=1 X=1 N=1 Z=0 V=0
; ==========================================================
    moveq   #2,d7
    moveq   #0,d0
    moveq   #1,d1
    sub.b   d1,d0
    bcc     .fail
    bpl     .fail
    beq     .fail
    bvs     .fail
    cmpi.b  #$ff,d0
    bne     .fail

; Check X=1 using SUBX
    moveq   #0,d2
    moveq   #0,d3
    subx.b  d3,d2
    cmpi.b  #$ff,d2
    bne     .fail
    rts

; ==========================================================
; TEST 3: SUB.B - zero result
; $01 - $01 = $00
; C=0 X=0 Z=1
; ==========================================================
    moveq   #3,d7
    moveq   #1,d0
    moveq   #1,d1
    sub.b   d1,d0
    bcs     .fail
    bne     .fail
    bmi     .fail
    bvs     .fail
    tst.b   d0
    bne     .fail

; ==========================================================
; TEST 4: SUB.B - signed overflow
; $80 - $01 = $7F
; V=1 N=0 C=0
; ==========================================================
    moveq   #4,d7
    moveq   #$80,d0
    moveq   #1,d1
    sub.b   d1,d0
    bvc     .fail
    bmi     .fail
    bcs     .fail
    beq     .fail
    cmpi.b  #$7f,d0
    bne     .fail

; ==========================================================
; TEST 5: SUB.B - opposite signed overflow
; $7F - $FF = $80
; V=1 N=1 C=1
; ==========================================================
    moveq   #5,d7
    moveq   #$7f,d0
    moveq   #-1,d1
    sub.b   d1,d0
    bvc     .fail
    bpl     .fail
    bcc     .fail
    beq     .fail
    cmpi.b  #$80,d0
    bne     .fail

; ==========================================================
; TEST 6: SUB.W - upper bits preserved
; $12340001 - $0001 = $12340000
; ==========================================================
    moveq   #6,d7
    move.l  #$12340001,d0
    moveq   #1,d1
    sub.w   d1,d0
    bcs     .fail
    bne     .fail
    bmi     .fail
    bvs     .fail
    cmpi.l  #$12340000,d0
    bne     .fail

; ==========================================================
; TEST 7: SUB.W - borrow, negative result
; $0000 - $0001 = $FFFF
; ==========================================================
    moveq   #7,d7
    move.l  #$12340000,d0
    moveq   #1,d1
    sub.w   d1,d0
    bcc     .fail
    bpl     .fail
    beq     .fail
    bvs     .fail
    cmpi.l  #$1234ffff,d0
    bne     .fail

; ==========================================================
; TEST 8: SUB.W - signed overflow
; $8000 - $0001 = $7FFF
; ==========================================================
    moveq   #8,d7
    move.w  #$8000,d0
    moveq   #1,d1
    sub.w   d1,d0
    bvc     .fail
    bmi     .fail
    bcs     .fail
    cmpi.w  #$7fff,d0
    bne     .fail

; ==========================================================
; TEST 9: SUB.L - borrow
; $00000000 - $00000001 = $FFFFFFFF
; ==========================================================
    moveq   #9,d7
    moveq   #0,d0
    moveq   #1,d1
    sub.l   d1,d0
    bcc     .fail
    bpl     .fail
    beq     .fail
    bvs     .fail
    cmpi.l  #$ffffffff,d0
    bne     .fail

; ==========================================================
; TEST 10: SUB.L - signed overflow
; $80000000 - $00000001 = $7FFFFFFF
; ==========================================================
    moveq   #10,d7
    move.l  #$80000000,d0
    moveq   #1,d1
    sub.l   d1,d0
    bvc     .fail
    bmi     .fail
    bcs     .fail
    cmpi.l  #$7fffffff,d0
    bne     .fail

; ==========================================================
; TEST 11: SUB.L - opposite signed overflow
; $7FFFFFFF - $FFFFFFFF = $80000000
; ==========================================================
    moveq   #11,d7
    move.l  #$7fffffff,d0
    moveq   #-1,d1
    sub.l   d1,d0
    bvc     .fail
    bpl     .fail
    bcc     .fail
    cmpi.l  #$80000000,d0
    bne     .fail

; ==========================================================
; TEST 12: SUBX.B - X=1 used as borrow input
; $00 - $00 - 1 = $FF
; ==========================================================
    moveq   #12,d7
    moveq   #0,d2
    moveq   #1,d3
    sub.b   d3,d2          ; Set X=1

    moveq   #0,d0
    moveq   #0,d1
    subx.b  d1,d0
    bcc     .fail
    bpl     .fail
    beq     .fail
    bvs     .fail
    cmpi.b  #$ff,d0
    bne     .fail

; ==========================================================
; TEST 13: SUBX.B - no borrow, X=0
; $02 - $01 - 0 = $01
; ==========================================================
    moveq   #13,d7
    moveq   #0,d2
    moveq   #0,d3
    sub.b   d3,d2          ; Clear X

    moveq   #2,d0
    moveq   #1,d1
    subx.b  d1,d0
    bcs     .fail
    bvs     .fail
    bmi     .fail
    beq     .fail
    cmpi.b  #1,d0
    bne     .fail

; ==========================================================
; TEST 14: SUBX.W - borrow across 16 bits
; $0000 - $0000 - X(1) = $FFFF
; ==========================================================
    moveq   #14,d7
    moveq   #0,d2
    moveq   #1,d3
    sub.b   d3,d2          ; Set X=1

    move.l  #$12340000,d0
    moveq   #0,d1
    subx.w  d1,d0
    bcc     .fail
    bpl     .fail
    beq     .fail
    bvs     .fail
    cmpi.l  #$1234ffff,d0
    bne     .fail

; ==========================================================
; TEST 15: SUBX.L - borrow across 32 bits
; $00000000 - $00000000 - X(1) = $FFFFFFFF
; ==========================================================
    moveq   #15,d7
    moveq   #0,d2
    moveq   #1,d3
    sub.b   d3,d2          ; Set X=1

    moveq   #0,d0
    moveq   #0,d1
    subx.l  d1,d0
    bcc     .fail
    bpl     .fail
    beq     .fail
    bvs     .fail
    cmpi.l  #$ffffffff,d0
    bne     .fail

; ==========================================================
; TEST 16: SUBX cumulative Z
; Result zero preserves previous Z
; ==========================================================
    moveq   #16,d7
    moveq   #0,d0
    moveq   #0,d1
    moveq   #0,d2
    moveq   #0,d3
    sub.b   d3,d2          ; X=0, Z=1

    subx.b  d1,d0
    bne     .fail          ; Z must remain 1

    subx.b  d1,d0
    bne     .fail          ; Still Z=1

; Previous Z=0, result zero must keep Z=0
    moveq   #1,d2          ; Z=0, X unchanged

    subx.b  d1,d0
    beq     .fail          ; Z must remain 0

    rts

.fail:
    nop                    ; Emulator stops here
    bra.s   .fail
