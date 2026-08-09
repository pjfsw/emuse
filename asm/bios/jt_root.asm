;____________________________________________________________
;
; Jump table for the root library
;____________________________________________________________

JT_ConClearLine:    ; -160
    jmp ConClearLine     
JT_ConClearEol:     ; -154
    jmp ConClearEol
JT_ConSetCursor:    ; -148
    jmp ConSetCursor
JT_ConCursorLeft:   ; -142
    jmp ConCursorLeft 
JT_ConCursorRight:  ; -136
    jmp ConCursorRight 
JT_ConCursorUp:     ; -130
    jmp ConCursorUp   
JT_ConCursorDown:   ; -124
    jmp ConCursorDown 
JT_ConUnderlinedText: ;-118
    jmp ConUnderlinedText
JT_ConReverseText: ; -112
    jmp ConReverseText
JT_ConBoldText:   ; -106
    jmp ConBoldText
JT_ConNormalText: ; -100
    jmp ConNormalText
JT_LibAdd:     ; -94
    jmp LMAddLibrary
JT_LibClose:   ; -88
    jmp LMCloseLibrary
JT_LibOpen:    ; -82
    jmp LMOpenLibrary
JT_MemTotal:   ; -76
    jmp MemTotal
JT_MemAvail:   ; -70
    jmp MemAvail
JT_MemFree:    ; -64
    jmp MemFree
JT_MemAlloc:   ; -58
    jmp MemAlloc
JT_ConGetChar: ; -52
    jmp ConGetChar
JT_ConReserved0 ; -46
    blk.b 6,0
JT_ConPutHex8:  ; -40
    jmp ConPutHex8
JT_ConPutHex16: ; -34
    jmp ConPutHex16
JT_ConPutHex32: ; -28
    jmp ConPutHex32
JT_ConPuts: ; -22
    jmp ConPuts
JT_ConPutc: ; -16
    jmp ConPutc
JT_ConClr: ; -10
    jmp ConClr
    dc.l ROOTLIB_VERSION      ; Version

