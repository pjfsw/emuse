TestAddAddx:
    movem.l d0-d3,-(sp)

; ==========================================================
; ADD.B - upper register bits must not affect carry
; ==========================================================
    move.l  #$0000db44,d0
    moveq   #10,d1
    add.b   d1,d0
    bcs     .fail
    bvs     .fail
    bmi     .fail
    beq     .fail
    cmpi.l  #$0000db4e,d0
    bne     .fail

; ==========================================================
; ADD.B - carry, extend and zero
; $FF + 1 = $00, C=1, X=1, Z=1
; ==========================================================
    moveq   #-1,d0
    moveq   #1,d1
    add.b   d1,d0
    bcc     .fail
    bne     .fail
    bvs     .fail
    bmi     .fail
    cmpi.l  #$ffffff00,d0
    bne     .fail

; Verify X=1 by adding it to zero
    moveq   #0,d0
    moveq   #0,d1
    addx.b  d1,d0
    cmpi.b  #1,d0
    bne     .fail

; ==========================================================
; ADD.B - positive overflow
; $7F + 1 = $80, V=1, N=1, C=0
; ==========================================================
    moveq   #$7f,d0
    moveq   #1,d1
    add.b   d1,d0
    bvc     .fail
    bpl     .fail
    bcs     .fail
    beq     .fail
    cmpi.b  #$80,d0
    bne     .fail

; ==========================================================
; ADD.B - negative overflow with carry
; $80 + $FF = $7F, V=1, C=1, N=0
; ==========================================================
    moveq   #$80,d0
    moveq   #-1,d1
    add.b   d1,d0
    bvc     .fail
    bcc     .fail
    bmi     .fail
    beq     .fail
    cmpi.b  #$7f,d0
    bne     .fail

; ==========================================================
; ADD.W - preserve upper 16 bits, no carry
; ==========================================================
    move.l  #$1234fffe,d0
    moveq   #1,d1
    add.w   d1,d0
    bcs     .fail
    bvs     .fail
    beq     .fail
    bpl     .fail
    cmpi.l  #$1234ffff,d0
    bne     .fail

; ==========================================================
; ADD.W - wraparound with carry
; ==========================================================
    move.l  #$1234ffff,d0
    moveq   #1,d1
    add.w   d1,d0
    bcc     .fail
    bne     .fail
    bvs     .fail
    cmpi.l  #$12340000,d0
    bne     .fail

; ==========================================================
; ADD.W - signed overflow
; ==========================================================
    move.w  #$7fff,d0
    moveq   #1,d1
    add.w   d1,d0
    bvc     .fail
    bpl     .fail
    bcs     .fail

; ==========================================================
; ADD.L - carry across 32 bits
; ==========================================================
    moveq   #-1,d0
    moveq   #1,d1
    add.l   d1,d0
    bcc     .fail
    bne     .fail
    bvs     .fail
    bmi     .fail
    tst.l   d0
    bne     .fail

; ==========================================================
; ADD.L - signed overflow
; ==========================================================
    move.l  #$7fffffff,d0
    moveq   #1,d1
    add.l   d1,d0
    bvc     .fail
    bpl     .fail
    bcs     .fail
    cmpi.l  #$80000000,d0
    bne     .fail

; ==========================================================
; ADD.L - negative + negative overflow
; ==========================================================
    move.l  #$80000000,d0
    moveq   #-1,d1
    add.l   d1,d0
    bvc     .fail
    bcc     .fail
    bmi     .fail
    cmpi.l  #$7fffffff,d0
    bne     .fail

; ==========================================================
; ADDX.B - carry input X=1
; 0 + 0 + X = 1
; ==========================================================
    moveq   #-1,d2
    moveq   #1,d3
    add.b   d3,d2           ; Set X=1

    moveq   #0,d0
    moveq   #0,d1
    addx.b  d1,d0
    bcs     .fail
    beq     .fail
    bmi     .fail
    bvs     .fail
    cmpi.b  #1,d0
    bne     .fail

; ==========================================================
; ADDX.B - carry output
; $FF + 1 + X(0) = $00
; ==========================================================
    moveq   #0,d2
    moveq   #0,d3
    add.b   d3,d2           ; Clear X

    moveq   #-1,d0
    moveq   #1,d1
    tst.b d2
    addx.b  d1,d0
    bcc     .fail
    bvs     .fail
    bmi     .fail
    bne     .fail

; ==========================================================
; ADDX.B - carry-in + operands generate carry
; $FF + $00 + X(1) = $00
; ==========================================================
    moveq   #-1,d0
    moveq   #0,d1
    addx.b  d1,d0
    bcc     .fail
    bne     .fail
    bvs     .fail

; ==========================================================
; ADDX.W - extend across 16 bits
; ==========================================================
    moveq   #-1,d2
    moveq   #1,d3
    add.b   d3,d2           ; Set X=1

    move.l  #$1234ffff,d0
    moveq   #0,d1
    addx.w  d1,d0
    bcc     .fail
    bvs     .fail
    bne     .fail
    cmpi.l  #$12340000,d0
    bne     .fail

; ==========================================================
; ADDX.L - extend across 32 bits
; ==========================================================
    moveq   #-1,d2
    moveq   #1,d3
    add.b   d3,d2           ; Set X=1

    moveq   #-1,d0
    moveq   #0,d1
    addx.l  d1,d0
    bcc     .fail
    bne     .fail
    bvs     .fail
    tst.l   d0
    bne     .fail

; ==========================================================
; ADDX - special cumulative Z behavior
; When result is zero, previous Z is preserved.
; ==========================================================

; Clear X, set Z=1
    moveq   #0,d2
    moveq   #0,d3
    add.b   d3,d2           ; X=0, Z=1

    moveq   #0,d0
    moveq   #0,d1
    addx.b  d1,d0           ; 0+0+0 = 0
    bne     .fail           ; Z must remain 1

    addx.b  d1,d0
    bne     .fail           ; Still Z=1

; Force previous Z=0 while keeping X=0
    moveq   #0,d0
    moveq   #0,d1
    moveq   #1,d2           ; Z=0, X unchanged

    addx.b  d1,d0           ; Result zero
    beq     .fail           ; Z must remain 0

; ==========================================================
; ADDX.B - signed overflow
; ==========================================================
    moveq   #0,d2
    moveq   #0,d3
    add.b   d3,d2           ; X=0

    moveq   #$7f,d0
    moveq   #1,d1
    addx.b  d1,d0
    bvc     .fail
    bpl     .fail
    bcs     .fail
    cmpi.b  #$80,d0
    bne     .fail

; ==========================================================
; All tests passed
; ==========================================================
    movem.l (sp)+,d0-d3
    rts

.fail:
    nop                     ; Breakpoint here
    bra.s   .fail

