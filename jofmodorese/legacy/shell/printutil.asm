PrintDecimalWord:
    lea DecBuffer(pc),a0    
    bsr PrintU16Decimal
    lea DecBuffer(pc),a1
    jmp CONPUTS(a6)

;____________________________________________________________
; D0 - Value to print
;____________________________________________________________
PrintDecimalLong:
    move.l d7,-(sp)
    lea DecBuffer(pc),a0    
    bsr PrintU32Decimal
    move.l a0,d0
    sub.l #DecBuffer,a0
    moveq #11,d7
    sub.l a0,d7
.pad:
    bsr PrintSpace
    dbra d7,.pad
    lea DecBuffer(pc),a1
    jsr CONPUTS(a6)
    move.l (sp)+,d7
    rts