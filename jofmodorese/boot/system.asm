;____________________________________________________________
;
; SYSTEM.BIN - System loader
;
; Loaded by the BIOS boot loader as a raw image at $1000.
; Loads the legacy shell (/SHELL.EXE, hunk format) and starts it.
;____________________________________________________________
    incdir "../include"
    include rootlib.i
    include psb.i
    include dirent.i

START equ $1000
    org START

    move.l ROOTLIB_BASE,a6

    lea ShellCtx(pc),a0
    lea ShellPath(pc),a1
    jsr DOS_CREATE_CONTEXT(a6)
    tst.l d0
    bne.s LoadFailed

    lea ShellCtx(pc),a0
    jsr DOS_LOAD_PROCESS(a6)
    tst.l d0
    bne.s LoadFailed

    ; A0: Process struct, first hunk is the entry point
    move.l ProcHunkStart(a0),a0
    lea ShellPsb(pc),a1     ; Empty process startup block in A1
    jsr (a0)

    move.l ROOTLIB_BASE,a6
    lea ShellExitMsg(pc),a1
    jsr CONPUTS(a6)
    bra.s HaltLoop

LoadFailed:
    move.l d0,d2
    move.l ROOTLIB_BASE,a6
    lea LoadFailedMsg(pc),a1
    jsr CONPUTS(a6)
    move.l d2,d0
    jsr CONPUTHEX32(a6)
    lea LineBreakMsg(pc),a1
    jsr CONPUTS(a6)
HaltLoop:
    bra.s HaltLoop

ShellPath:
    dc.b "/SHELL.EXE",0
LoadFailedMsg:
    dc.b 13,10,"Failed to load /SHELL.EXE, error ",0
ShellExitMsg:
    dc.b 13,10,"Shell exited, system halted",13,10,0
LineBreakMsg:
    dc.b 13,10,0
    even
ShellCtx:
    blk.b PCTX_SIZEOF,0
ShellPsb:
    blk.b PsbSizeOf,0
