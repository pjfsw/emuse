    include rootlib.i
    include errcode.i

START equ $1000
    org START


MAX_CMDLINE_LENGTH equ 128
    ; End of BIOS init code        
    move.l ROOTLIB_BASE,a6    
    lea DosLoadingMsg(pc),a1
    jsr CONPUTS(a6)
    move.l #DOS_LIB_ID,d0
    moveq #1,d1
    jsr LIBOPEN(a6)
    tst.l d0
    bpl.s .dosLibOk
    rts
.dosLibOk:
    lea DosLibBase(pc),a0
    move.l d0,(a0)
    move.l (a0),a5    

    move.l #READBUFFER_SIZE,d0
    jsr MEMALLOC(a6)
    lea ReadBufferPtr(pc),a0
    move.l d0,(a0)

    lea LineBreakMsg(pc),a1
    jsr CONPUTS(a6)
    bsr ClearCommandLine
    lea CommandLine(pc),a4  ; Command line buffer
MainLoop:    
    bsr PrintPrompt
.waitForChar:    
    jsr CONGETC(a6)
    tst.l d0
    bmi.s .waitForChar    
    cmp.b #$7f,d0
    beq.s .eraseChar
    cmp.b #13,d0
    beq.s .lineBreak
    cmp.w #MAX_CMDLINE_LENGTH-1,d6
    bhs.s .waitForChar
    cmp.b #32,d0
    blo.s .waitForChar
    cmp.b #127,d0
    bhi.s .waitForChar
    move.b d0,(a4,d6.w)    
    addq.w #1,d6
    jsr CONPUTC(a6) ; CONPUTC
    bra.s .waitForChar
.lineBreak:
    lea LineBreakMsg(pc),a1
    jsr CONPUTS(a6)
    bsr ParseCommandLine
    bsr ClearCommandLine
    bra MainLoop    
.eraseChar:
    tst.w d6
    beq.s .waitForChar
    subq.w #1,d6
    clr.b (a4,d6.w)
    move.b #8,d0
    jsr CONPUTC(a6)
    move.b #32,d0
    jsr CONPUTC(a6)
    move.b #8,d0
    jsr CONPUTC(a6)
    bra .waitForChar

; Check internal command as provided in A0 with the command line provided in A1
CheckInternalCommand:
.compare:
    move.b (a0)+,d0
    beq.s .commandNameEnd
    
    cmp.b (a1)+,d0
    bne.s .noMatch
    bra.s .compare
.commandNameEnd:
    move.b (a1),d0
    beq.s .match
    cmp.b #' ',d0
    bne.s .noMatch
.match:
    moveq #0,d0
    rts
.noMatch:
    moveq #-1,d0
    rts

ParseCommandLine:
    movem.l a2/a3,-(sp)
    bsr.s .parseCommandLine
    movem.l (sp)+,a2/a3
    rts
.parseCommandLine:    
    lea CommandLine(pc),a1
    bsr TrimLeadingSpaces
    tst.b (a1)
    bne.s .cmdLineNotEmpty
    rts
.cmdLineNotEmpty:
    move.l a1,a3
.findEnd:
    move.b (a1),d0
    beq.s .endOfCmdLine
    lea 1(a1),a1
    bra .findEnd
.endOfCmdLine:
    move.b -1(a1),d0
    cmp.b #' ',d0
    bne.s .trimmed
    clr.b -1(a1)
    lea -1(a1),a1
    bra .endOfCmdLine
.trimmed:    
    lea BuiltInCommands(pc),a2
.nextCommand:
    tst.l (a2)
    beq.s .notBuiltInCommand
    move.l (a2),a0
    move.l a3,a1
    bsr CheckInternalCommand
    tst.l d0
    beq.s .internalCommandFound
    lea 4(a2),a2
    bra.s .nextCommand
.notBuiltInCommand:
    move.l a3,a1
    bsr ExecuteCommand
    tst.l d0
    bne PrintError
    rts

.internalCommandFound:
    bsr TrimLeadingSpaces
    move.l 4(a2),a2 ; Jump vector
    jsr (a2)
    tst.l d0
    bne PrintError
    rts

; Move pointer A1 to first non space character
TrimLeadingSpaces:
    move.b (a1),d0
    beq.s .endOfString
    cmp.b #' ',d0
    bne.s .endOfString
    adda.l #1,a1
    bra.s TrimLeadingSpaces    
.endOfString:
    rts

PrintPrompt:
    lea MsgPrompt1(pc),a1
    jsr CONPUTS(a6)    
    jsr CONBOLD(a6)
    lea CurrentDir,a1
    jsr CONPUTS(a6)
    jsr CONNORMAL(a6)
    lea MsgPrompt2(pc),a1
    jmp CONPUTS(a6) ; CONPUTS


ClearCommandLine:
    moveq #MAX_CMDLINE_LENGTH/4-1,d7
    lea CommandLine(pc),a0
.clearCmdLine:
    clr.l (a0)+
    dbra d7,.clearCmdLine
    moveq #0,d6     ; Command line position    

    rts

PrintErrorCode:
    jsr CONPUTHEX32(a6)
    lea LineBreakMsg(pc),a1
    jmp CONPUTS(a6)

MsgPrompt1:
    dc.b "[",0
MsgPrompt2:
    dc.b "]$ ",0
    even

PrintSpace:
    move.b #' ',d0
    jmp CONPUTC(a6)

PrintEightChars:
    moveq #7,d7
.nextChar:
    move.b (a1)+,d0
    move.l a1,-(sp)
    jsr CONPUTC(a6)
    move.l (sp)+,a1
    dbra d7,.nextChar
    rts

PrintTextAndNumber:
    move.l d0,-(sp)
    jsr CONPUTS(a6)
    move.l (sp)+,d0
    jsr CONPUTHEX32(a6)
    lea LineBreakMsg(pc),a1
    jmp CONPUTS(a6)

BuiltInCommands:
    dc.l CommandLs,ExecuteLs
    dc.l CommandCd,ExecuteCd
    dc.l CommandCat,ExecuteCat
    dc.l CommandFree,ExecuteFree
    dc.l CommandPart,ExecutePart
    dc.l 0,0
CommandLs:
    dc.b "ls",0
CommandCd:
    dc.b "cd",0
CommandCat:
    dc.b "cat",0
CommandFree:
    dc.b "free",0
CommandPart:
    dc.b "part",0
DosLoadingMsg:
    dc.b 13,10,13,10,"Starting JOFMODORE DOS 1.0...",0
PathErrorMsg:
    dc.b "Invalid path",0
LineBreakMsg:
    dc.b 13,10,0    
    even

DosLibBase:
    dc.l 0
READBUFFER_SIZE EQU 2048
ReadBufferPtr:
    dc.l 0

DecBuffer:
    ds.b 12,0

    include decimal.asm
    include cd.asm
    include ls.asm
    include cat.asm
    include errcode.asm
    include exec.asm
    include free.asm
    include part.asm
    include printutil.asm

CurrentDir:
    dc.b "/",0
    blk.b 10,0

CommandLine:    blk.b MAX_CMDLINE_LENGTH,0
ShellPartitionInfo: blk.b 32,0
DirectoryCtx: blk.b PCTX_SIZEOF,0
DirEntry: blk.b DIRENT_SIZEOF,0

