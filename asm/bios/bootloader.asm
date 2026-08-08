    incdir include
    include "rootlib.i"
    include "doslib.i"
BootLoader:
    lea OSVARS_BASE,a0
    tst.w OsBootMediaStatus(a0)
    beq.s .bootMediaInitialized
    move.l ROOTLIB_BASE,a6
    lea .bootMediaNotFoundMsg,a1
    jmp CONPUTS(a6)
.bootMediaInitialized:
    move.l ROOTLIB_BASE,a6
    move.l #DOS_LIB_ID,d0
    move.l #DOS_LIB_VERSION,d1
    jsr LIBOPEN(a6)
    move.l d0,a6
    lea OSVARS_BASE,a0
    lea OsDosState(a0),a0
    lea DosPathContext(a0),a2
    move.l a2,a0
    lea .systemFilename,a1
    jsr DOS_CREATE_CONTEXT(a6)   
    tst.l d0
    bmi.s .bootFailure
    
    ; Load binary into RAM
    move.l a2,a0
    lea BOOT_LOADER_BASE,a1
    move.l PCTX_BYTES_REM(a2),d0
    move.l d0,d2
    jsr DOS_READ_FILE(a6)
    cmp.l d0,d2
    bne.s .bootFailure
    add.l #BOOT_LOADER_BASE,d0
    bsr MemInit
    jmp BOOT_LOADER_BASE
.bootFailure:
    move.l d0,d2
    move.l ROOTLIB_BASE,a6
    lea .loadErrorMsg(pc),a1
    jsr CONPUTS(a6)
    move.l d2,d0
    jsr CONPUTHEX32(a6)
    lea LineBreakMsg,a1
    jsr CONPUTS(a6)
    rts
.bootMediaNotFoundMsg:
    dc.b "No boot device found.",0
.loadErrorMsg:
    dc.b "Boot failure: ",0
.systemFilename:
    dc.b "/SYSTEM.BIN",0
    even    