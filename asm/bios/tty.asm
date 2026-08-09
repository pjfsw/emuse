    include "osvars.i"

TTYInit:
    move.w #$4ef9,d0    ; JMP instruction
    move.l #TTYDummy,d1
    lea OSVARS_BASE+OsConsoleFunc,a0
    move.l a0,a1
    move.w #ConsoleFuncSizeof/6-1,d7
.fillJumps:
    move.w d0,(a1)+
    move.l d1,(a1)+
    dbra d7,.fillJumps
    
    move.l #UARTPutChar,2+ConsolePutcFunc(a0)   
    move.l #UARTGetChar,2+ConsoleGetcFunc(a0)
    move.l #TTYPuts,2+ConsolePutsFunc(a0)
    move.l #TTYClear,2+ConsoleClearFunc(a0)
    move.l #TTYNormalText,2+ConsoleNormalTextFunc(a0)
    move.l #TTYBoldText,2+ConsoleBoldTextFunc(a0)
    move.l #TTYReverseText,2+ConsoleReverseTextFunc(a0)
    move.l #TTYUnderlinedText,2+ConsoleUnderlinedTextFunc(a0)
    move.l #TTYCursorDown,2+ConsoleCurDnFunc(a0)
    move.l #TTYCursorUp,2+ConsoleCurUpFunc(a0)
    move.l #TTYCursorRight,2+ConsoleCurRtFunc(a0)
    move.l #TTYCursorLeft,2+ConsoleCurLtFunc(a0)
    move.l #TTYSetCursor,2+ConsoleCursorFunc(a0)
    move.l #TTYClearEol,2+ConsoleClearEolFunc(a0)
    move.l #TTYClearLine,2+ConsoleClearLineFunc(a0)

    bra UARTInit

TTYDummy:
    moveq #0,d0
    rts

TTYPutc:
    lea OSVARS_BASE+OsConsoleFunc,a0
    jmp ConsolePutcFunc(a0)

; A1 Pointer to string
TTYPuts:
    move.l a2,-(sp)
    lea OSVARS_BASE+OsConsoleFunc,a2
.next:
    move.b (a1)+,d0
    beq.s .done
    jsr ConsolePutcFunc(a2)
    bra.s .next
.done:
    move.l (sp)+,a2
    rts    

TTYClear:
    lea .msg(pc),a1
    bra TTYPuts
.msg:  
    dc.b 27,"[2J",27,"[H",0
    even

TTYBoldText:
    lea .msg(pc),a1
    bra TTYPuts
.msg:
    dc.b 27,"[1m",0
    even

TTYNormalText:
    lea .msg(pc),a1
    bra TTYPuts
.msg:
    dc.b 27,"[0m",0
    even

TTYReverseText:
    lea .msg(pc),a1
    bra TTYPuts
.msg:
    dc.b 27,"[7m",0
    even

TTYUnderlinedText:
    lea .msg(pc),a1
    bra TTYPuts
.msg:
    dc.b 27,"[4m",0
    even

TTYClearEol:
    lea .msg(pc),a1
    bra TTYPuts
.msg:
    dc.b 27,"[K",0
    even

TTYClearLine:
    lea .msg(pc),a1
    bra TTYPuts
.msg:
    dc.b 27,"[2K",0
    even

; D0 = row, D1 = col
TTYSetCursor:
    move.l d1,-(sp)
    move.l d0,-(sp)
    lea EscapeCode(pc),a1
    bsr TTYPuts
    move.l (sp)+,d0
    bsr TTYWriteNumber
    move.b #';',d0
    bsr TTYPutc
    move.l (sp)+,d0
    bsr TTYWriteNumber
    move.b #'H',d0
    bra TTYPutc        

TTYCursorUp:
    bsr.s TTYMoveCursor
    move.b #'A',d0
    bra TTYPutc

TTYCursorDown:
    bsr.s TTYMoveCursor
    move.b #'B',d0
    bra TTYPutc

TTYCursorLeft:
    bsr.s TTYMoveCursor
    move.b #'D',d0
    bra TTYPutc

TTYCursorRight:
    bsr.s TTYMoveCursor
    move.b #'C',d0
    bra TTYPutc

TTYMoveCursor:
    move.l d0,-(sp)
    lea EscapeCode(pc),a1
    bsr TTYPuts
    move.l (sp)+,d0
    bra TTYWriteNumber

TTYWriteNumber:
    move.l d0,-(sp)
    bsr TTYWriteHiNibble
    move.l (sp)+,d0
    bra TTYWriteLoNibble

TTYWriteHiNibble:
    and.l #$ff,d0
    cmp.b #9,d0
    bls.s .done
    lea .hinibbles(pc),a0
    move.b (a0,d0.w),d0
    bra TTYPutc
.done:
    rts
.hinibbles:
    dc.b '0','0','0','0','0','0','0','0','0','0'
    dc.b '1','1','1','1','1','1','1','1','1','1'
    dc.b '2','2','2','2','2','2','2','2','2','2'
    dc.b '3','3','3','3','3','3','3','3','3','3'
    dc.b '4','4','4','4','4','4','4','4','4','4'
    dc.b '5','5','5','5','5','5','5','5','5','5'
    dc.b '6','6','6','6','6','6','6','6','6','6'
    dc.b '7','7','7','7','7','7','7','7','7','7'
    dc.b '8','8','8','8','8','8','8','8','8','8'
    dc.b '9','9','9','9','9','9','9','9','9','9'

TTYWriteLoNibble:
    and.l #$ff,d0
    lea .lonibbles(pc),a0
    move.b (a0,d0.w),d0
    bra TTYPutc
.lonibbles:
    dc.b '0','1','2','3','4','5','6','7','8','9'
    dc.b '0','1','2','3','4','5','6','7','8','9'
    dc.b '0','1','2','3','4','5','6','7','8','9'
    dc.b '0','1','2','3','4','5','6','7','8','9'
    dc.b '0','1','2','3','4','5','6','7','8','9'
    dc.b '0','1','2','3','4','5','6','7','8','9'
    dc.b '0','1','2','3','4','5','6','7','8','9'
    dc.b '0','1','2','3','4','5','6','7','8','9'
    dc.b '0','1','2','3','4','5','6','7','8','9'
    dc.b '0','1','2','3','4','5','6','7','8','9'

EscapeCode:
    dc.b 27,"[",0
    even

    include uart.asm
