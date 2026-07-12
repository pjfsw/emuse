HUNK_HEADER equ $3f3

    include "osvars.i"
    include "errcode.i"

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
FMLoadExecutable:
    movem.l a3-a6/d7,-(sp)
    bsr .loadExecutableInt
    movem.l (sp)+,a3-a6/d7
    rts
.loadExecutableInt:
    move.l DosLibBase,a6
    move.l a0,a4    ; Path context
    jsr GetProcDosState
    move.l a0,a5    ; DOS context
    clr.w DosStreamOffset(a5)
    move.l a4,a0
    lea DosBuffer(a5),a1
    move.l #512,d0
    jsr DOS_READ_FILE(a6)
    tst.l d0    
    beq.s .invalidExe
    bpl.s .fileOk
    rts
.fileOk:
    cmp.l #31,d0    ; Sanity check
    bhi.s .bigEnough   
.invalidExe:    
    suba.l a0,a0
    move.l #DOS_ERR_NOT_EXECUTABLE,d0
    rts
.bigEnough:
    lea DosBuffer(a5),a0
    cmp.l #HUNK_HEADER,(a0)+
    bne.s .invalidExe
    tst.l (a0)+ ; Check string list, we only support empty list
    bne.s .invalidExe
    move.l (a0)+,d7 ; Table size
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
    rts

File
    include fileman.asm

