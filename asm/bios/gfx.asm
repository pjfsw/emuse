    include "osvars.i"
    include "hardware.i"

GfxScrollOffset  EQU 2
GfxTextY         EQU 4

GfxInit:    
    lea OSVARS_BASE+OsConsoleFunc,a0
    move.l #GfxPutChar,2+ConsolePutcFunc(a0)
    move.l #GfxNormalText,2+ConsoleNormalTextFunc(a0)
    move.l #GfxBoldText,2+ConsoleBoldTextFunc(a0)
    move.l #GfxClear,2+ConsoleClearFunc(a0)
    move.l #GfxMoveLeft,2+ConsoleCurLtFunc(a0)
    lea GFXBASE,a0
    lea GfxFont,a1        
    move.b #$0f,GFXFONTBANK(a0) ; Font at 0xF000
    clr.b GFXADDRLO(a0)
    move.b #$f0,GFXADDRHI(a0)
    move.w #255*4,d0
.copyFont:
    move.b (a1)+,GFXDATA(a0)
    move.b (a1)+,GFXDATA(a0)
    move.b (a1)+,GFXDATA(a0)
    move.b (a1)+,GFXDATA(a0)
    dbra d0,.copyFont

    ; RRGGGBBM
    ;move.b #$00,GFXCOL0R(a0)
    ;move.b #$52,GFXCOL1R(a0)  
    ;move.b #$fe,GFXCOL2R(a0)
    ;move.b #$f6,GFXCOL3R(a0)
    bra GfxClear

GfxPutChar:
    movem.l d0-d7/a0-a6,-(sp)
    bsr.s .gfxPutChar
    movem.l (sp)+,d0-d7/a0-a6
    rts
.gfxPutChar:    
    lea OSVARS_BASE,a2
    lea OsGfxVars(a2),a2
    lea GFXBASE,a1
    cmp.b #8,d0
    beq.s .backspace
    cmp.b #10,d0
    beq.s .lineBreak
    cmp.b #13,d0
    beq.s .done
    move.w (a2),d1

.writeChar:
    ; Set destination address
    move.b d1,GFXADDRLO(a1)
    lsr.w #8,d1
    move.b d1,GFXADDRHI(a1)

    move.b d0,GFXDATA(a1)

 ; Advance cursor
    addq.w #1,(a2)

    ; Wrap at column 80
    move.w (a2),d1
    and.w #$7F,d1
    cmp.w #80,d1
    beq.s .lineBreak

.done:
    rts

.backspace:
    move.w (a2),d1
    subq.w #1,d1
    and.w #$FFF,d1
    move.w d1,d2
    and.w #$7F,d2
    cmp.w #80,d2
    blo.s .backspaceWrite

    sub.w #48,d1              ; $7F -> $4F (column 79)    
.backspaceWrite:
    move.b #' ',d0
    move.w d1,(a2)
    move.b d1,GFXADDRLO(a1)
    lsr.w #8,d1
    move.b d1,GFXADDRHI(a1)
    move.b d0,GFXDATA(a1)
    rts

.lineBreak:
    move.w (a2),d1
    add.w #$80,d1
    and.w #$0F80,d1
    move.w d1,(a2)
    ; Round up to next $80-byte VRAM row
    move.w d1,d0
    move.b d0,GFXADDRLO(a1)
    lsr.w #8,d0
    move.b d0,GFXADDRHI(a1)

    moveq #15,d7   ; 128/8 = 16
.clrLine:
    clr.b GFXDATA(a1)            
    clr.b GFXDATA(a1)            
    clr.b GFXDATA(a1)            
    clr.b GFXDATA(a1)            
    clr.b GFXDATA(a1)            
    clr.b GFXDATA(a1)            
    clr.b GFXDATA(a1)            
    clr.b GFXDATA(a1)            
    dbra d7,.clrLine

    move.b d1,GFXADDRLO(a1)
    lsr.w #8,d1
    move.b d1,GFXADDRHI(a1)

    ; Check scrolling
    move.w GfxTextY(a2),d1
    addq.w #1,d1
    cmp.w #30,d1
    blo.s .noScroll

    ; First scroll occurs when TextY reaches 30
    move.w d1,d2
    sub.w #29,d2
    and.w #31,d2

    ; Convert character row to scroll register units
    lsl.w #3,d2
    move.b d2,GFXSCROLLY(a1)
.noScroll:    
    move.w d1,GfxTextY(a2)
    rts

GfxBoldText:
    move.l a1,-(sp)
    lea GFXBASE,a1
    move.b #$0F,GFXATTR(a1)
    move.l a1,(sp)+
    rts

GfxNormalText:
    move.l a1,-(sp)
    lea GFXBASE,a1
    move.b #$07,GFXATTR(a1)
    move.l a1,(sp)+
    rts

GfxMoveLeft:
    movem.l d0-d7/a0-a6,-(sp)
    bsr.s .gfxMoveLeft
    movem.l (sp)+,d0-d7/a0-a6
    rts
.gfxMoveLeft:    
    lea OSVARS_BASE,a2
    lea OsGfxVars(a2),a2
    lea GFXBASE,a1
    move.w (a2),d1
    subq.w #1,d1
    and.w #$FFF,d1
    move.w d1,(a2)
    move.b d1,GFXADDRLO(a1)
    lsr.w #8,d1
    move.b d1,GFXADDRHI(a1)
    rts

GfxClear:
    move.l d7,-(sp)
    bsr.s .gfxClear
    move.l (sp)+,d7
    rts
.gfxClear:    
    lea GFXBASE,a0
    clr.b GFXADDRHI(a0)
    clr.b GFXADDRLO(a0)
    moveq #0,d0
    move.w #2047,d7
.clrVram:    
    move.b d0,GFXDATA(a0)
    move.b d0,GFXDATA(a0)
    dbra d7,.clrVram
    clr.b GFXADDRHI(a0)
    clr.b GFXADDRLO(a0)
    move.b #$07,GFXATTR(a0)    
    clr.b GFXSCROLLY(a0)    

    lea OSVARS_BASE+OsGfxVars,a1
    clr.w (a1)
    clr.w GfxScrollOffset(a1)
    clr.w GfxTextY(a1)
    rts

GfxFont:
    INCBIN "IBM_VGA_8x16.bin"