    include "osvars.i"

ConOpen:
    lea OSVARS_BASE+OsConsoleFunc,a0
    bra TTYInit

;____________________________________________________________
;
; Clear screen
;____________________________________________________________
ConClr: 
    lea OSVARS_BASE+OsConsoleFunc,a0
    jmp ConsoleClearFunc(a0)

;____________________________________________________________
;
; Write character (byte) in D0 to console
;____________________________________________________________
ConPutc:
    lea OSVARS_BASE+OsConsoleFunc,a0
    jmp ConsolePutcFunc(a0)

;____________________________________________________________
;
; Write a null-terminated string (A1) to console.
;____________________________________________________________
ConPuts:
    lea OSVARS_BASE+OsConsoleFunc,a0
    jmp ConsolePutsFunc(a0)

ConPutHexNibble:
    and.b #15,d0
    add.b #'0',d0
    cmp.b #':',d0
    bcs.s .1
    add.b #7,d0
.1:    
    bra ConPutc

;____________________________________________________________
;
; Write a 32-bit hex value (D0) to console.
;____________________________________________________________
ConPutHex32:
    movem.l d2/d7,-(sp)
    move.l d0,d2   
    moveq #8,d7
.1:
    rol.l #4,d2
    move.b d2,d0
    bsr ConPutHexNibble
    subq #1,d7    
    bne.s .1

    movem.l (sp)+,d2/d7
    rts

;____________________________________________________________
;
; Write a 16-bit hex value (D0) to console.
;____________________________________________________________
ConPutHex16:
    movem.l d2/d7,-(sp)
    move.w d0,d2   
    moveq #4,d7
.1:
    rol.w #4,d2
    move.b d2,d0
    bsr ConPutHexNibble
    subq #1,d7    
    bne.s .1

    movem.l (sp)+,d2/d7
    rts

;____________________________________________________________
;
; Write a 8-bit hex value (D0) to console.
;____________________________________________________________
ConPutHex8:
    movem.l d2/d7,-(sp)
    move.w d0,d2   
    moveq #2,d7
.1:
    rol.b #4,d2
    move.b d2,d0
    bsr ConPutHexNibble
    subq #1,d7    
    bne.s .1

    movem.l (sp)+,d2/d7
    rts

;____________________________________________________________
;
; Read character from console
; Return character 0-255 in D0, or -1 if no char available
;____________________________________________________________
ConGetChar:
    lea OSVARS_BASE+OsConsoleFunc,a0
    jmp ConsoleGetcFunc(a0)

;____________________________________________________________
;
; Set normal text
;____________________________________________________________
ConNormalText:
    lea OSVARS_BASE+OsConsoleFunc,a0
    jmp ConsoleNormalTextFunc(a0)

;____________________________________________________________
;
; Set bold text
;____________________________________________________________
ConBoldText:
    lea OSVARS_BASE+OsConsoleFunc,a0
    jmp ConsoleBoldTextFunc(a0)

;____________________________________________________________
;
; Set reverse text
;____________________________________________________________
ConReverseText:
    lea OSVARS_BASE+OsConsoleFunc,a0
    jmp ConsoleReverseTextFunc(a0)

;____________________________________________________________
;
; Set underlined text
;____________________________________________________________
ConUnderlinedText:
    lea OSVARS_BASE+OsConsoleFunc,a0
    jmp ConsoleUnderlinedTextFunc(a0)

ConCursorUp:
    lea OSVARS_BASE+OsConsoleFunc,a0
    jmp ConsoleCurUpFunc(a0)

ConCursorDown:
    lea OSVARS_BASE+OsConsoleFunc,a0
    jmp ConsoleCurDnFunc(a0)

ConCursorLeft:
    lea OSVARS_BASE+OsConsoleFunc,a0
    jmp ConsoleCurLtFunc(a0)

ConCursorRight:
    lea OSVARS_BASE+OsConsoleFunc,a0
    jmp ConsoleCurRtFunc(a0)

    include "tty.asm"