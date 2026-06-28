    include hardware.i

NOT_A_DIGIT equ $FFFF

Uploader:
    lea Uploader_StartMsg(pc),a1
    bsr ConPuts

    lea SIMPLE_LOADER_BASE,a2       ; Loader v0.9 loads everything into address $10000
    moveq #0,d2                     ; D2 is the current digit
    move.w #2,d3                    ; Digits remaining for the next byte
.nextNibble:
    bsr ConGetChar
    tst.l d0
    bmi.s .nextNibble
    cmp.b #27,d0
    beq.s .abort    
    cmp.b #'.',d0    
    beq.s .done
    and.w #$ff,d0
    bsr ConvertDigit
    cmp.w #NOT_A_DIGIT,d0
    beq.s .nextNibble

    rol.b #4,d2
    or.b d0,d2   
    subq.w #1,d3
    bne.s .nextNibble    
    move.b d2,(a2)+
    moveq #0,d2
    move.w #2,d3
    bra.s .nextNibble

.abort:
    lea Uploader_AbortedMsg(pc),a1
    bsr ConPuts
    rts
.done:
    lea Uploader_DoneMsg(pc),a1
    bsr ConPuts
    move.l #SIMPLE_LOADER_BASE,d0
    bsr ConPutHex32
    move.b #'-',d0
    bsr ConPutc
    move.l a2,d0
    bsr ConPutHex32
    lea Uploader_DoneMsg2(pc),a1
    bsr ConPuts
    jsr SIMPLE_LOADER_BASE
    ; Todo execute
    rts

; Convert digit in D0 to nibble
ConvertDigit:
    cmp.b #'0',d0
    bcs.s .isInvalid

    cmp.b #'9',d0
    bcs.s .isDigit
    beq.s .isDigit

    cmp.b #'A',d0
    bcs.s .isInvalid

    cmp.b #'G',d0      ; one past 'F'
    bcc.s .isInvalid      ; >= 'G'

    ; A-F
    sub.b #'A'-10,d0
    bra.s .isDone
.isDigit:
    sub.b #'0',d0
.isDone:
    rts
.isInvalid:
    move.w #NOT_A_DIGIT,d0
    rts
    
Uploader_StartMsg:
    dc.b 13,10,"Paste S-record data into terminal now.",13,10
    dc.b "Press <ESC> to abort: ",0
Uploader_AbortedMsg:
    dc.b "aborted.",13,10,0    
Uploader_DoneMsg:
    dc.b "Upload complete! (",0    
Uploader_DoneMsg2:
    dc.b ")",13,10,0
    even    

    include console.asm