    incdir ..
    incdir ../../include
    include hardware.i
    include rootlib.i

SNES_LATCH    equ (1<<0)
SNES_CLOCK    equ (1<<1)
SNES_DATA_BIT equ 4
SNES_BUTTONS  equ 12
LAYOUT_ROWS   equ 11

    ; ~6us at 14.7 MHz (moveq 4 + 9*10 + 14 cycles)
    macro SnesDelay
    moveq #8,d0
.delay\@:
    dbra d0,.delay\@
    endm

;START equ $1000
    ;org START

    code

    move.l ROOTLIB_BASE,a6
    jsr CONCLR(a6)
    lea Layout(pc),a1
    jsr CONPUTS(a6)

    lea SnesPrev,a0                    ; force first draw of all buttons
    moveq #SNES_BUTTONS-1,d0
.initPrev:
    move.b #$ff,(a0)+
    dbra d0,.initPrev

.flushChars:
    jsr CONGETC(a6)
    tst.l d0
    bpl.s .flushChars

loop:
    bsr ReadController
    bsr DrawButtons
    jsr CONGETC(a6)                    ; any key exits
    tst.l d0
    bmi.s loop

    moveq #LAYOUT_ROWS+1,d0            ; leave cursor below layout
    moveq #1,d1
    jsr CONSETCRS(a6)
    moveq #0,d0
    rts

;____________________________________________________________
;
; DrawButtons - Update pressed markers that changed since
;               the last call
; A6: ROOTLIB base
;____________________________________________________________
DrawButtons:
    movem.l d2-d4/a2-a4,-(sp)
    lea SnesButtons,a2
    lea SnesPrev,a3
    lea ButtonPos(pc),a4
    moveq #0,d4                        ; set if anything was drawn
    moveq #SNES_BUTTONS-1,d2
.nextButton:
    move.b (a2)+,d3
    cmp.b (a3),d3
    beq.s .unchanged
    move.b d3,(a3)
    moveq #0,d0
    moveq #0,d1
    move.b (a4),d0                     ; row
    move.b 1(a4),d1                    ; column
    jsr CONSETCRS(a6)
    moveq #' ',d0
    tst.b d3
    beq.s .put
    moveq #'X',d0
.put:
    jsr CONPUTC(a6)
    moveq #1,d4
.unchanged:
    addq.l #1,a3
    addq.l #2,a4
    dbra d2,.nextButton

    tst.b d4
    beq.s .done
    moveq #LAYOUT_ROWS+1,d0            ; park cursor below layout
    moveq #1,d1
    jsr CONSETCRS(a6)
.done:
    movem.l (sp)+,d2-d4/a2-a4
    rts

ReadController:
    movem.l d2/a2,-(sp)
    ; IREG bit 4 is SNES controller data 
    ; OREG+SNES bit 0 is SNES controller latch
    ; OREG+SNES bit 1 is SNES controller clock
    lea IREG,a0
    lea OREG+SNES,a1
    lea SnesButtons,a2

    ; Both 74HC259 outputs are written on every access,
    ; so clock must be kept high while toggling latch.
    move.b #SNES_LATCH|SNES_CLOCK,(a1) ; latch high, clock idle high
    SnesDelay
    SnesDelay                          ; ~12us latch pulse
    move.b #SNES_CLOCK,(a1)            ; latch low, B is now on data
    SnesDelay

    ; Order: B, Y, Select, Start, Up, Down, Left, Right, A, X, L, R
    moveq #SNES_BUTTONS-1,d1
.nextButton:
    move.b (a0),d2
    lsr.b #SNES_DATA_BIT,d2            ; data bit to bit 0
    andi.b #1,d2
    eori.b #1,d2                       ; active low: 1 = pressed, 0 = released
    move.b d2,(a2)+
    move.b #0,(a1)                     ; clock low
    ;SnesDelay
    move.b #SNES_CLOCK,(a1)            ; rising edge shifts next bit
    ;SnesDelay
    dbra d1,.nextButton

    movem.l (sp)+,d2/a2
    rts

; Row, column (1-based) of the marker inside "( )", in SnesButtons order
ButtonPos:
    dc.b 8,40                          ; B
    dc.b 7,38                          ; Y
    dc.b 9,27                          ; Select
    dc.b 9,31                          ; Start
    dc.b 6,15                          ; Up
    dc.b 8,15                          ; Down
    dc.b 7,13                          ; Left
    dc.b 7,17                          ; Right
    dc.b 7,42                          ; A
    dc.b 6,40                          ; X
    dc.b 3,11                          ; L
    dc.b 3,49                          ; R

Layout:
    dc.b "  SNES controller test - press any key to exit",13,10
    dc.b 13,10
    dc.b "       L ( )                                 R ( )",13,10
    dc.b "     .-------------------------------------------.",13,10
    dc.b "    /        Up                        X          ",92,13,10
    dc.b "   |         ( )                      ( )          |",13,10
    dc.b "   |  Left ( ) ( ) Right          Y ( ) ( ) A      |",13,10
    dc.b "   |         ( )                      ( )          |",13,10
    dc.b "   |        Down  Select ( ) ( ) Start  B          |",13,10
    dc.b "    ",92,"                                             /",13,10
    dc.b "     '-------------------------------------------'",13,10
    dc.b 0
    even

    bss
SnesButtons: ds.b SNES_BUTTONS
SnesPrev:    ds.b SNES_BUTTONS
    


