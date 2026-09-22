    include "osvars.i"
    include "hardware.i"

GfxInit:    
    lea OSVARS_BASE+OsConsoleFunc,a0
    move.l #GfxPutChar,2+ConsolePutcFunc(a0)
    move.l #GfxNormalText,2+ConsoleNormalTextFunc(a0)
    move.l #GfxBoldText,2+ConsoleBoldTextFunc(a0)
    move.l #GfxClear,2+ConsoleClearFunc(a0)
    lea GFXBASE,a0
    ; RRGGGBBM
    move.b #$00,GFXCOL0(a0)
    move.b #$52,GFXCOL1(a0)  
    move.b #$fe,GFXCOL2(a0)
    move.b #$f6,GFXCOL3(a0)
    bra GfxClear

GFX_BLOCK_HEIGHT_SHIFT  equ 3
GFX_BLOCK_HEIGHT        equ (1<<GFX_BLOCK_HEIGHT_SHIFT)
GFX_COLUMNS             equ 80
GFX_STRIDE              equ 128
GFX_BYTES_PER_BLOCK_ROW equ GFX_STRIDE*GFX_BLOCK_HEIGHT


GfxPutChar:
    movem.l d0-d7/a0-a6,-(sp)
    bsr.s .gfxPutChar
    movem.l (sp)+,d0-d7/a0-a6
    rts
.gfxPutChar:    
    lea OSVARS_BASE,a2
    lea OsGfxVars(a2),a2
    lea GFXBASE,a1
    cmp.b #10,d0
    beq.s .lineBreak
    cmp.b #13,d0
    bne.s .ok
    rts
.ok:    
    and.l #$ff,d0
    lsl.w #GFX_BLOCK_HEIGHT_SHIFT,d0
    lea GfxFont(pc),a0
    add.l d0,a0
    moveq #GFX_BLOCK_HEIGHT-1,d7
.renderChar:
    move.b (a0)+,GFXDATA(a1)
    dbra d7,.renderChar

    move.w (a2),d1
    add.w #GFX_BLOCK_HEIGHT,d1            ; Skip the bytes we just rendered
    move.w d1,(a2)
    and.w #GFX_BYTES_PER_BLOCK_ROW-1,d1    ; offset within 2048 4-byte character row
    cmp.w #GFX_COLUMNS*GFX_BLOCK_HEIGHT,d1          ; 80 * 16
    bne.s .noWrap
    bra.s .lineBreak
.noWrap:
    rts

.lineBreak:
    move.w (a2),d0
    ; Round up to next $400-byte VRAM row
    add.w #GFX_BYTES_PER_BLOCK_ROW,d0
    and.w #-(GFX_BYTES_PER_BLOCK_ROW),d0
    move.w d0,(a2)
    move.w d0,d1
    move.b d0,GFXADDRLO(a1)
    lsr.w #8,d0
    move.b d0,GFXADDRHI(a1)

    move.b #0,d0
    move.w #(GFX_BYTES_PER_BLOCK_ROW/8)-1,d7
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

    rts

GfxBoldText:
    rts

GfxNormalText:
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
    move.w #32767,d7
.clrVram:    
    move.b d0,GFXDATA(a0)
    move.b d0,GFXDATA(a0)
    dbra d7,.clrVram
    clr.b GFXADDRHI(a0)
    clr.b GFXADDRLO(a0)
    lea OSVARS_BASE,a1
    clr.w OsGfxVars(a1)
    rts

GfxFont:
    INCBIN "font.bin"