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

    bra UARTInit

TTYDummy:
    moveq #0,d0
    rts

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
    lea (.msg).l,a1
    bra TTYPuts
.msg:  
    dc.b 27,"[2J",27,"[H",0
    even

TTYBoldText:
    lea (.msg).l,a1
    bra TTYPuts
.msg:
    dc.b 27,"[1m",0
    even

TTYNormalText:
    lea (.msg).l,a1
    bra TTYPuts
.msg:
    dc.b 27,"[0m",0
    even

TTYReverseText:
    lea (.msg).l,a1
    bra TTYPuts
.msg:
    dc.b 27,"[7m",0
    even

TTYUnderlinedText:
    lea (.msg).l,a1
    bra TTYPuts
.msg:
    dc.b 27,"[4m",0
    even

    include uart.asm
