HUNK_HEADER equ $3f3
HUNK_CODE   equ $3e9
HUNK_DATA   equ $3ea

    include "osvars.i"
    include "errcode.i"



;____________________________________________________________
; Requirements: A4 contains PathCtx, A5 contains temp long buffer, D0 number of bytes (max 16)
FMStreamRead:
    move.l d2,-(sp)
    bsr .fmStreamReadInt
    move.l (sp)+,d2
    rts
.fmStreamReadInt:    
    move.l d0,d2
    move.l a4,a0
    move.l a5,a1    
    jsr DOS_READ_FILE(a6)
    tst.l d0
    bpl.s .notReadError
    rts
.notReadError:
    cmp.l d0,d2
    bne.s .fail
    moveq #0,d0
    rts
.fail:
    moveq #DOS_ERR_NOT_EXECUTABLE,d0   
    rts


;____________________________________________________________
;
; Load executable
;
; A0: pointer to path context 
; Return: D0: 0 upon success, otherwise error code
;         A0: pointer to process
;____________________________________________________________
FMLoadExecutable:
    movem.l a2-a6/d5-d7,-(sp)
    bsr.s .loadExecutableInt
    movem.l (sp)+,a2-a6/d5-d7
    rts
.loadExecutableInt:    
    move.l DosLibBase,a6
    move.l a0,a4    ; Path context
    jsr GetProcDosState
    move.l a0,a5    ; DOS context
    lea DosTemp(a5),a5  ; Temp long word

    moveq #8,d0       
    bsr FMStreamRead
    bpl.s .ok1
    rts
.ok1:
    cmp.l #HUNK_HEADER,(a5)
    bne .invalidExe

    tst.l 4(a5)     ; No support for strings
    bne .invalidExe

    moveq #12,d0
    bsr FMStreamRead  ; Read hunk sizes
    bpl.s .ok4
    rts
.ok4:
    move.l (a5),d6     ; Table size
    move.l d6,d7
    subq.l #1,d7
    tst.l 4(a5)
    bne.s .invalidExe   ; Only support first hunk=0
    cmp.l 8(a5),d7
    bne.s .invalidExe   ; Only support last hunk=size-1

    moveq #0,d5
.calcAllocSize:
    moveq #4,d0
    bsr FMStreamRead
    bne.s .invalidExe
    add.l (a5),d5               ; TODO WE NEED TO KEEP TRACK OF EACH HUNK START FOR the relocation later
    dbra d7,.calcAllocSize

    move.l d5,d0
    lsl.l #2,d0
    add.l #ProcSizeof,d0
    bsr MemAlloc
    tst.l d0
    bne.s .memoryOk
    moveq #DOS_ERR_OUT_OF_MEMORY,d0
    rts
.memoryOk:
    move.l d0,a2        ; PROCESS NOW IN A2!!
    move.w d6,ProcHunkCount(a2)
    move.l d6,d7
    subq.w #1,d7
    lea ProcSizeof(a2),a1
    moveq #0,d0
    rts
.invalidExe:
    moveq #DOS_ERR_NOT_EXECUTABLE,d0
    rts
File
    include fileman.asm

