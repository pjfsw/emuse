;____________________________________________________________
;
; Serial loader
;
; Receives a hunk executable over the console as hex digits and
; loads it while it arrives: hunk memory is allocated from the
; sizes in the hunk header, code/data are written directly to
; their final location and relocations are applied as they are
; received. No copy of the file is kept in memory.
;
; Stream format: the executable file as hex digits (0-9, A-F,
; a-f). All other characters are ignored, '.' ends the stream
; and ESC aborts. Output of tools/mkhex.py can be sent as is.
;
; Supported hunks: CODE, DATA, BSS, RELOC32,
;                  RELOC32SHORT/DREL32 and END
;____________________________________________________________

HUNK_CODE           equ $3e9
HUNK_DATA           equ $3ea
HUNK_BSS            equ $3eb
HUNK_RELOC32        equ $3ec
HUNK_END            equ $3f2
HUNK_HEADER         equ $3f3
HUNK_DREL32         equ $3f7
HUNK_RELOC32SHORT   equ $3fc

SL_MAX_HUNKS        equ 4

SL_ERR_ABORTED      equ -1      ; ESC received
SL_ERR_END          equ -2      ; '.' received before executable was complete
SL_ERR_FORMAT       equ -3      ; Invalid or unsupported executable
SL_ERR_MEMORY       equ -4      ; Not enough memory

;____________________________________________________________
;
; Receive a hunk executable over serial, run it and free its
; memory when it returns
;
; Input:  -
; Output: -
;____________________________________________________________
SerialLoadAndRun:
    move.l ROOTLIB_BASE,a6
    lea SlSendMsg(pc),a1
    jsr CONPUTS(a6)

    bsr SlReceive
    tst.l d0
    bne.s .failed

    bsr SlSkipToEnd         ; Consume the rest of the stream up to '.'
    bsr SlDrain

    lea SlLoadedMsg(pc),a1
    jsr CONPUTS(a6)
    move.l SlTotal,d0
    jsr CONPUTHEX32(a6)
    lea SlAtMsg(pc),a1
    jsr CONPUTS(a6)
    move.l SlBlock,d0
    jsr CONPUTHEX32(a6)
    lea LineBreakMsg(pc),a1
    jsr CONPUTS(a6)

    move.l SlHunkAddrs,a0   ; Entry point is the start of the first hunk
    move.l SlBlock,a1
    bra RunAndFree

.failed:
    move.l d0,d2            ; Error code in D2
    tst.l SlBlock
    beq.s .noFree
    move.l SlBlock,a0
    jsr MEMFREE(a6)
.noFree:
    cmp.l #SL_ERR_ABORTED,d2
    beq.s .streamDone
    cmp.l #SL_ERR_END,d2
    beq.s .streamDone
    bsr SlSkipToEnd         ; Discard the rest of the stream
.streamDone:
    bsr SlDrain
    move.l d2,d0
    lea SlFailedMsg(pc),a1
    bra PrintError

;____________________________________________________________
;
; Receive and load a hunk executable
;
; Input:  -
; Output: D0: 0 on success, < 0 on error (SL_ERR_*)
;         SlBlock:     allocated memory (0 if none)
;         SlTotal:     size of allocated memory
;         SlHunkAddrs: start address of each hunk
;
; Register usage:
;   D3: Size of current hunk in bytes
;   D4: Base address of relocation target hunk
;   D5: Current hunk index
;   D6: Number of hunks
;   D7: Loop counter
;   A3: Base address of current hunk
;____________________________________________________________
SlReceive:
    clr.l SlBlock

    bsr SlGetLong
    bne .ioError
    cmp.l #HUNK_HEADER,d0
    bne .format
    bsr SlGetLong           ; Resident library names, not supported
    bne .ioError
    tst.l d0
    bne .format
    bsr SlGetLong           ; Number of hunks
    bne .ioError
    move.l d0,d6
    beq .format
    cmp.l #SL_MAX_HUNKS,d6
    bhi .format
    bsr SlGetLong           ; First hunk must be 0
    bne .ioError
    tst.l d0
    bne .format
    bsr SlGetLong           ; Last hunk must be count-1
    bne .ioError
    addq.l #1,d0
    cmp.l d6,d0
    bne .format

    ; Read hunk sizes, total size in D7
    lea SlHunkSizes(pc),a2
    moveq #0,d7
    move.l d6,d5
    subq.l #1,d5
.readSize:
    bsr SlGetLong
    bne .ioError
    move.l d0,d1
    and.l #$c0000000,d1
    cmp.l #$c0000000,d1     ; Extended memory attributes, not supported
    beq .format
    and.l #$3fffffff,d0     ; Ignore memory type flags
    lsl.l #2,d0
    move.l d0,(a2)+
    add.l d0,d7
    dbra d5,.readSize

    ; Allocate memory for all hunks
    move.l d7,SlTotal
    beq .format
    move.l d7,d0
    jsr MEMALLOC(a6)
    tst.l d0
    beq .memory
    move.l d0,SlBlock

    ; Clear all hunk memory, this also initializes BSS
    move.l d0,a3
    move.l d7,d1
    lsr.l #2,d1
.clear:
    clr.l (a3)+
    subq.l #1,d1
    bne.s .clear

    ; Calculate start address of each hunk
    lea SlHunkSizes(pc),a0
    lea SlHunkAddrs(pc),a1
    move.l SlBlock,d1
    move.l d6,d5
    subq.l #1,d5
.calcAddr:
    move.l d1,(a1)+
    add.l (a0)+,d1
    dbra d5,.calcAddr

    moveq #0,d5
.nextHunk:
    move.l d5,d0
    lsl.l #2,d0
    lea SlHunkAddrs(pc),a0
    move.l (a0,d0.l),a3
    lea SlHunkSizes(pc),a0
    move.l (a0,d0.l),d3
.nextBlock:
    bsr SlGetLong
    bne .ioError
    cmp.l #HUNK_CODE,d0
    beq.s .codeData
    cmp.l #HUNK_DATA,d0
    beq.s .codeData
    cmp.l #HUNK_BSS,d0
    beq.s .bss
    cmp.l #HUNK_RELOC32,d0
    beq.s .reloc32
    cmp.l #HUNK_RELOC32SHORT,d0
    beq .relocShort
    cmp.l #HUNK_DREL32,d0
    beq .relocShort
    cmp.l #HUNK_END,d0
    beq .hunkEnd
    bra .format

.codeData:
    bsr SlGetLong           ; Number of longwords
    bne .ioError
    lsl.l #2,d0
    cmp.l d3,d0
    bhi .format
    move.l d0,d7
    beq.s .nextBlock
    move.l a3,a2
.copyByte:
    bsr SlGetByte
    bmi .done
    move.b d0,(a2)+
    subq.l #1,d7
    bne.s .copyByte
    bra.s .nextBlock

.bss:
    bsr SlGetLong           ; Number of longwords, memory already cleared
    bne .ioError
    lsl.l #2,d0
    cmp.l d3,d0
    bhi .format
    bra .nextBlock

.reloc32:
    bsr SlGetLong           ; Number of offsets, 0 terminates
    bne .ioError
    move.l d0,d7
    beq .nextBlock
    bsr SlGetLong           ; Target hunk
    bne .ioError
    bsr SlTargetBase
    bne .format
.reloc32Offset:
    bsr SlGetLong
    bne .ioError
    bsr SlRelocate
    bne .format
    subq.l #1,d7
    bne.s .reloc32Offset
    bra.s .reloc32

.relocShort:
    moveq #0,d2             ; Number of words read, for padding
.shortGroup:
    bsr SlGetWord           ; Number of offsets, 0 terminates
    bne .ioError
    addq.l #1,d2
    move.l d0,d7
    beq.s .shortDone
    bsr SlGetWord           ; Target hunk
    bne .ioError
    addq.l #1,d2
    bsr SlTargetBase
    bne .format
.shortOffset:
    bsr SlGetWord
    bne .ioError
    addq.l #1,d2
    bsr SlRelocate
    bne .format
    subq.l #1,d7
    bne.s .shortOffset
    bra.s .shortGroup
.shortDone:
    btst #0,d2              ; Block is padded to a longword boundary
    beq .nextBlock
    bsr SlGetWord
    bne .ioError
    bra .nextBlock

.hunkEnd:
    addq.l #1,d5
    cmp.l d6,d5
    blo .nextHunk
    moveq #0,d0
.done:
    rts
.ioError:
    move.l d1,d0
    rts
.format:
    moveq #SL_ERR_FORMAT,d0
    rts
.memory:
    moveq #SL_ERR_MEMORY,d0
    rts

;____________________________________________________________
;
; Get base address of relocation target hunk
;
; Input:  D0: Target hunk index
;         D6: Number of hunks
; Output: D4: Base address of target hunk
;         D0: 0 if OK (Z set), < 0 if invalid hunk index
;____________________________________________________________
SlTargetBase:
    cmp.l d6,d0
    bhs.s .invalid
    lsl.l #2,d0
    lea SlHunkAddrs(pc),a0
    move.l (a0,d0.l),d4
    moveq #0,d0
    rts
.invalid:
    moveq #-1,d0
    rts

;____________________________________________________________
;
; Relocate one longword in the current hunk
;
; Input:  D0: Offset in current hunk
;         D3: Size of current hunk
;         D4: Base address of target hunk
;         A3: Base address of current hunk
; Output: D0: 0 if OK (Z set), < 0 if offset is invalid
;____________________________________________________________
SlRelocate:
    btst #0,d0              ; Must be word aligned on 68000
    bne.s .invalid
    move.l d0,d1
    addq.l #4,d1
    cmp.l d3,d1
    bhi.s .invalid
    add.l d4,0(a3,d0.l)
    moveq #0,d0
    rts
.invalid:
    moveq #-1,d0
    rts

;____________________________________________________________
;
; Read a big endian longword from the stream
;
; Output: D0: Value
;         D1: 0 if OK (Z set), < 0 on error (SL_ERR_*)
;____________________________________________________________
SlGetLong:
    move.l d2,-(sp)
    moveq #0,d2
    bsr.s SlGetByte
    bmi.s .error
    move.b d0,d2
    bsr.s SlGetByte
    bmi.s .error
    lsl.l #8,d2
    move.b d0,d2
    bsr.s SlGetByte
    bmi.s .error
    lsl.l #8,d2
    move.b d0,d2
    bsr.s SlGetByte
    bmi.s .error
    lsl.l #8,d2
    move.b d0,d2
    move.l d2,d0
    move.l (sp)+,d2
    moveq #0,d1
    rts
.error:
    move.l d0,d1
    move.l (sp)+,d2
    tst.l d1
    rts

;____________________________________________________________
;
; Read a big endian word from the stream
;
; Output: D0: Value 0-65535
;         D1: 0 if OK (Z set), < 0 on error (SL_ERR_*)
;____________________________________________________________
SlGetWord:
    move.l d2,-(sp)
    bsr.s SlGetByte
    bmi.s .error
    move.l d0,d2
    lsl.l #8,d2
    bsr.s SlGetByte
    bmi.s .error
    or.l d2,d0
    move.l (sp)+,d2
    moveq #0,d1
    rts
.error:
    move.l d0,d1
    move.l (sp)+,d2
    tst.l d1
    rts

;____________________________________________________________
;
; Read a byte (two hex digits) from the stream
;
; Output: D0: Byte 0-255, or < 0 on error (SL_ERR_*), N flag set
;____________________________________________________________
SlGetByte:
    bsr.s SlGetNibble
    bmi.s .done
    move.w d0,-(sp)
    bsr.s SlGetNibble
    bmi.s .error
    move.w (sp)+,d1
    lsl.b #4,d1
    or.b d1,d0
    tst.l d0
.done:
    rts
.error:
    addq.l #2,sp
    tst.l d0
    rts

;____________________________________________________________
;
; Read a hex digit from the stream, other characters are ignored
;
; Output: D0: Value 0-15, or < 0 on ESC/'.' (SL_ERR_*), N flag set
;____________________________________________________________
SlGetNibble:
    jsr CONGETC(a6)
    tst.l d0
    bmi.s SlGetNibble
    cmp.b #27,d0
    beq.s .abort
    cmp.b #'.',d0
    beq.s .end
    cmp.b #'0',d0
    blo.s SlGetNibble
    cmp.b #'9',d0
    bls.s .digit
    and.b #$df,d0           ; To upper case
    cmp.b #'A',d0
    blo.s SlGetNibble
    cmp.b #'F',d0
    bhi.s SlGetNibble
    sub.b #'A'-10,d0
    bra.s .valid
.digit:
    sub.b #'0',d0
.valid:
    and.l #$f,d0
    rts
.abort:
    moveq #SL_ERR_ABORTED,d0
    rts
.end:
    moveq #SL_ERR_END,d0
    rts

;____________________________________________________________
;
; Skip the stream up to and including '.' (or ESC)
;____________________________________________________________
SlSkipToEnd:
    bsr.s SlGetNibble
    bpl.s SlSkipToEnd
    rts

;____________________________________________________________
;
; Discard characters until the line has been quiet for a while
;____________________________________________________________
SlDrain:
    move.l d3,-(sp)
.wait:
    move.l #20000,d3        ; About 25 ms at 14.7 MHz
.delay:
    subq.l #1,d3
    bne.s .delay
    jsr CONGETC(a6)
    tst.l d0
    bpl.s .wait
    move.l (sp)+,d3
    rts

SlSendMsg:
    dc.b 13,10,"Send hex encoded executable now, end with '.' (ESC aborts)",13,10,0
SlLoadedMsg:
    dc.b "Loaded $",0
SlAtMsg:
    dc.b " bytes at $",0
SlFailedMsg:
    dc.b "Serial load failed, error ",0
    even
SlBlock:
    dc.l 0
SlTotal:
    dc.l 0
SlHunkSizes:
    blk.l SL_MAX_HUNKS,0
SlHunkAddrs:
    blk.l SL_MAX_HUNKS,0
