EXE_NOT_AN_EXECUTABLE equ $ffff0000
HUNK_HEADER equ $3f3


;____________________________________________________________
;
; Read a long word from stream. 
StreamGetLong:

;____________________________________________________________
;
; Load executable
;
; A0: pointer to path context 
; Return: D0: 0 upon success, otherwise error code
;         A0: pointer to first code entry
;____________________________________________________________
LoadExe:
    movem.l a3-a6/d6-d7,-(sp)
    bsr .loadExeInt
    move.l (sp)+,a3-a6/d6-d7
.loadExeInt:
    lea OSVARS_BASE,a6
    move.l a0,a4    ; Path context
    jsr GetProcDosState
    move.l a0,a5    ; DOS context
    clr.l DOSINFO_STREAM_OFS(a5)

    lea DosBuffer(a5),a1
    move.l #512,d0
    bsr FMReadFile
    tst.l d0
    beq.s .invalidExe
    cmp.l #31,d0    ; Sanity check
    bhi.s .bigEnough
    beq.s .invalidExe    
.invalidExe:    
    suba.l a0,a0
    move.l #EXE_NOT_AN_EXECUTABLE,d0
    rts
.bigEnough:
    move.l a3,a0
    cmp.l #HUNK_HEADER,(a0)+
    bne.s .invalidExe
    tst.l (a0)+ ; Check string list, we only support empty list
    bne.s .invalidExe
    move.l (a0)+,d6 ; Table size
    move.l d6,d7
    subq.l #1,d7
    tst.l (a0)+
    bne.s .invalidExe   ; Only support first hunk=0
    cmp.l (a0)+,d7
    bne.s .invalidExe   ; Only support last hunk=size-1
    move.l a0,a3        ; Hunk size
    moveq #0,d0
.calcHunks:
    add.l (a0)+,d0
    dbra d7,.calcHunks            

    moveq #0,d0
    rts

    include fileman.asm

