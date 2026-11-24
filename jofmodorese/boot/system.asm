;____________________________________________________________
;
; SYSTEM.BIN - Boot menu
;
; Loaded by the BIOS boot loader as a raw image at $1000.
; Lets the user select which hunk executable to load and start:
;   1) The legacy shell (/SHELL.EXE), default
;   2) The new kernel   (/KERNEL.EXE)
;   3) A hunk executable received over the serial port,
;      hex encoded and terminated by '.' (see serload.asm)
;
; If loading fails, or the started program returns, the menu
; is shown again.
;____________________________________________________________
    incdir "../include"
    include rootlib.i
    include psb.i
    include dirent.i

START equ $1000
    org START

    ; The BIOS allocator can never free the first block allocated
    ; after MemInit. Allocate a small permanent block here, so the
    ; memory of programs started from the menu can be freed.
    move.l ROOTLIB_BASE,a6
    moveq #4,d0
    jsr MEMALLOC(a6)

BootMenu:
    move.l ROOTLIB_BASE,a6
    lea MenuMsg(pc),a1
    jsr CONPUTS(a6)
.waitKey:
    jsr CONGETC(a6)
    tst.l d0
    bmi.s .waitKey
    cmp.b #13,d0
    beq.s .legacy
    cmp.b #'1',d0
    beq.s .legacy
    cmp.b #'2',d0
    beq.s .kernel
    cmp.b #'3',d0
    beq.s .serial
    bra.s .waitKey
.legacy:
    lea ShellPath(pc),a1
    bra.s .boot
.kernel:
    lea KernelPath(pc),a1
.boot:
    bsr LoadAndRun
    bra.s BootMenu
.serial:
    bsr SerialLoadAndRun
    bra.s BootMenu

;____________________________________________________________
;
; Load a hunk executable and run it
;
; Input:  A1: Path to executable (zero-terminated)
; Output: -
;____________________________________________________________
LoadAndRun:
    move.l a1,a3            ; Path in A3
    move.l ROOTLIB_BASE,a6
    lea LoadingMsg(pc),a1
    jsr CONPUTS(a6)
    move.l a3,a1
    jsr CONPUTS(a6)
    lea LineBreakMsg(pc),a1
    jsr CONPUTS(a6)

    lea ProgCtx(pc),a0
    move.l a3,a1
    jsr DOS_CREATE_CONTEXT(a6)
    tst.l d0
    bne.s .loadFailed

    lea ProgCtx(pc),a0
    jsr DOS_LOAD_PROCESS(a6)
    tst.l d0
    bne.s .loadFailed

    ; A0: Process struct, first hunk is the entry point
    move.l a0,a1            ; Memory to free afterwards
    move.l ProcHunkStart(a0),a0
    bra.s RunAndFree

.loadFailed:
    lea LoadFailedMsg(pc),a1
    bra.s PrintError

;____________________________________________________________
;
; Run a loaded program and free its memory when it returns
;
; Input:  A0: Entry point
;         A1: Allocated memory to free when the program returns
; Output: -
;____________________________________________________________
RunAndFree:
    move.l a1,-(sp)         ; Save memory block
    lea ProgPsb(pc),a1      ; Empty process startup block in A1
    jsr (a0)
    move.l (sp)+,a0

    move.l ROOTLIB_BASE,a6
    jsr MEMFREE(a6)         ; Free program memory
    lea ExitMsg(pc),a1
    jsr CONPUTS(a6)
    rts

;____________________________________________________________
;
; Print error message followed by error code
;
; Input:  A1: Message
;         D0: Error code
; Output: -
;____________________________________________________________
PrintError:
    move.l d0,d2
    move.l ROOTLIB_BASE,a6
    jsr CONPUTS(a6)
    move.l d2,d0
    jsr CONPUTHEX32(a6)
    lea LineBreakMsg(pc),a1
    jsr CONPUTS(a6)
    rts

    include serload.asm

ShellPath:
    dc.b "/SHELL.EXE",0
KernelPath:
    dc.b "/KERNEL.EXE",0
MenuMsg:
    dc.b 13,10,"Jofmodore SixtyEight boot menu",13,10
    dc.b "  1) Legacy shell",13,10
    dc.b "  2) Kernel",13,10
    dc.b "  3) Receive executable over serial",13,10
    dc.b "Select [1]: ",0
LoadingMsg:
    dc.b 13,10,"Loading ",0
LoadFailedMsg:
    dc.b "Load failed, error ",0
ExitMsg:
    dc.b 13,10,"Program exited",13,10,0
LineBreakMsg:
    dc.b 13,10,0
    even
ProgCtx:
    blk.b PCTX_SIZEOF,0
ProgPsb:
    blk.b PsbSizeOf,0
