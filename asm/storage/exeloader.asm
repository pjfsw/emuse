HUNK_END    equ $3f2
HUNK_HEADER equ $3f3
HUNK_CODE   equ $3e9
HUNK_DATA   equ $3ea
HUNK_BSS    equ $3eb
HUNK_DREL32 equ $3f7
HUNK_RELOC32SHORT equ $3fC

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
    cmp.l #PROC_MAX_HUNKS,d6
    bhi .invalidExe    ; We only support 3 hunks

    move.l d6,d7
    subq.l #1,d7
    tst.l 4(a5)
    bne .invalidExe   ; Only support first hunk=0
    cmp.l 8(a5),d7
    bne .invalidExe   ; Only support last hunk=size-1

    lea DOS_TEMP_AREA_SIZE(a5),a2   ; Hunk offset temp storage
    moveq #0,d5
.calcAllocSize:
    moveq #4,d0    
    bsr FMStreamRead
    bne .invalidExe
    move.l d5,(a2)+             ; Store zero based offset of hunks
    add.l (a5),d5               
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
    move.l d0,a2                ; PROCESS NOW IN A2!!
    move.w d6,ProcHunkCount(a2)  
    lea ProcSizeof(a2),a1     ; Start of hunks
    move.l a1,d1
    lea ProcHunkStart(a2),a1
    move.l d6,d7
    subq.w #1,d7
    lea DOS_TEMP_AREA_SIZE(a5),a0    
.updateHunkOffsets:
    move.l (a0)+,d0  ; Zero based start of this hunk
    lsl.l #2,d0
    add.l d1,d0
    move.l d0,(a1)+
    dbra d7,.updateHunkOffsets      
    lea ProcHunkStart(a2),a3    ; HUNK start in a3!
    moveq #0,d5                 ; Current hunk offset in d5
.readNextHunk:
    moveq #4,d0
    bsr FMStreamRead
    bne.s .invalidExe
    move.l (a5),d0
    cmp.l #HUNK_CODE,d0
    beq.s .readCodeOrData
    cmp.l #HUNK_DATA,d0
    beq.s .readCodeOrData
    cmp.l #HUNK_DREL32,d0
    beq.s .relocateShort
    cmp.l #HUNK_END,d0
    beq .hunkEnd
    cmp.l #HUNK_BSS,d0
    beq .hunkBss
    move.l (a5),d0
    ;moveq #0,d0
    move.l a2,a0
    rts
.success:
    move.l a2,a0
    moveq #0,d0
    rts
.invalidExe:
    moveq #DOS_ERR_NOT_EXECUTABLE,d0
    rts
.readCodeOrData:
    moveq #4,d0
    bsr FMStreamRead
    bne.s .invalidExe
    move.l (a5),d0 ; Number of long words to read
    lsl.l #2,d0    
    move.l a4,a0
    move.l (a3,d5.w),a1
    jsr DOS_READ_FILE(a6)    
    tst.l d0
    bpl.s .readNextHunk
    rts

.relocateShort:
    move.l a4,-(sp)
    bsr.s .relocateShort2
    move.l (sp)+,a4
    rts
.relocateShort2:
    moveq #2,d0
    bsr FMStreamRead
    bne.s .invalidExe
    move.w (a5),d7  ; Number of words
    beq.s .readNextHunk
    moveq #2,d0
    bsr FMStreamRead
    bne.s .invalidExe
    moveq #0,d0
    move.w (a5),d0     ; Get hunk that is referenced for reallocation
    lsl.l #2,d0        ; Scale it by 4 
    move.l (a3,d0.l),d6  ; Base address of referenced hunk and store in D6

    subq.w #1,d7
.nextWord:
    moveq #2,d0
    bsr FMStreamRead
    bne.s .invalidExe
    move.l (a3,d5.w),a0    ; A3+D5 is the pointer to the current hunk
    moveq #0,d0
    move.w (a5),d0    ; Offset in current hunk
    move.l (a0,d0.w),d1 ; Value to change
    add.l d6,d1
    move.l d1,(a0,d0.w)
    dbra d7,.nextWord
    bra.s .relocateShort
    bra .readNextHunk

.hunkEnd:
    moveq #0,d0
    move.w ProcHunkCount(a2),d0
    lsl.l #2,d0
    addq.w #4,d5
    cmp.w d0,d5
    beq.s .success
    bra .readNextHunk

.hunkBss:
    moveq #4,d0
    bsr FMStreamRead
    bne .invalidExe

    move.l (a5),d7                 ; Number of longwords
    move.l (a3,d5.w),a0
.clearBss:
    tst.l d7
    beq .readNextHunk

    clr.l (a0)+
    subq.l #1,d7
    bra.s .clearBss

    include fileman.asm
