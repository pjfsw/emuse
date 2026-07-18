HUNK_HEADER equ $3f3
HUNK_CODE   equ $3e9
HUNK_DATA   equ $3ea

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
    movem.l a2-a6/d6-d7,-(sp)
    bsr.s .loadExecutableInt
    movem.l (sp)+,a2-a6/d6-d7
    rts
.loadExecutableInt:
    move.l DosLibBase,a6
    move.l a0,a4    ; Path context
    jsr GetProcDosState
    move.l a0,a5    ; DOS context
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
    move.l (a0)+,d6     ; Table size
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
    lsl.l #2,d0         ; Long words
    add.l #ProcSizeof,d0
    bsr MemAlloc
    tst.l d0
    bne.s .memoryOk
    moveq #DOS_ERR_OUT_OF_MEMORY,d0
    rts
.memoryOk:
    move.l d0,a2        ; PROCESS NOW IN A2!!
    move.l a3,a0        ; Hunk sizes
    move.w d6,ProcHunkCount(a2)
    move.l d6,d7
    subq.w #1,d7
    lea ProcSizeof(a2),a1
    moveq #0,d0
.updateProcessHunks:
    move.l a1,ProcHunkStart(a2,d0.w)       
    move.l (a0)+,d1
    lsl.l #2,d1
    move.l d1,ProcHunkSize(a2,d0.w)
    adda.l d1,a1
    addq.w #4,d0
    dbra d7,.updateProcessHunks

    move.l a0,a1
    lea DosBuffer(a5),a0    
    suba.l a0,a1
    move.l a1,d0
    lsr.l #2,d0
    move.w d0,ProcStreamOffset(a2)  ; Starting now we poll long words until EOF or unexpected hunk
    clr.w ProcCurrentHunk(a2)
.nextLong:
    bsr StreamGetNextLong
    tst.l d0
    bmi .invalidExe
    rts
    cmp.l #HUNK_CODE,

    rts

File
    include fileman.asm

