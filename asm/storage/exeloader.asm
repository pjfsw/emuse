EXE_NOT_AN_EXECUTABLE equ $ffff0000
HUNK_HEADER equ $3f3
;____________________________________________________________
;
; Load executable
;
; A1: filename
;
; A0: allocated address or 0 if failure
; A1: entry point or 0 if failure
; D0: 0 upon success, otherwise error code
;____________________________________________________________
LoadExe:
    movem.l a2-a3,-(sp)
    bsr .loadExeInt
    move.l (sp)+,a2-a3
.loadExeInt:
    lea ChangeToSomePathContext,a0
    lea ChangeToSomeSectorBuffer,a3
    move.l a0,a2
    bsr FMCreateContext
    tst.l d0
    beq.s .contextOk
    rts
.contextOk:
    move.l a2,a0
    move.l a3,a1
    move.l #512,d0
    bsr FMReadFile
    tst.l d0
    beq.s .invalidExe
    cmp.l #31,d0    ; Sanity check
    bhi.s .bigEnough
    suba.l a0,a0
    suba.l a1,a1
    rts
.invalidExe:    
    move.l #EXE_NOT_AN_EXECUTABLE,d0
    rts
.bigEnough:
    move.l a3,a0
    cmp.l #HUNK_HEADER,(a0)
    bne.s .invalidExe

    moveq #0,d0
    rts

    include fileman.asm

