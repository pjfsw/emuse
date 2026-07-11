    incdir "../lib"
    include "errcode.i"

PrintError:
    movem.l d2/a2/a6,-(sp)
    bsr .printError
    movem.l (sp)+,d2/a2/a6
    rts
.printError:
    move.l ROOTLIB_BASE,a6
    lea .errorTable(pc),a0
.findErrorCode:
    tst.l (a0)
    beq.s .genericError
    cmp.l (a0),d0
    beq.s .specificError
    lea 8(a0),a0
    bra.s .findErrorCode
.specificError:       
    move.l 4(a0),a2
    lea .errorMsg(pc),a1
    jsr CONPUTS(a6)
    move.l a2,a1
    jsr CONPUTS(a6)
    lea LineBreakMsg,a1
    jmp CONPUTS(a6)
.genericError:
    move.l d0,d2
    lea .genericErrorMsg,a1
    jsr CONPUTS(a6)
    move.l d2,d0
    jsr CONPUTHEX32(a6)
    lea LineBreakMsg,a1
    jmp CONPUTS(a6)
.errorTable:
    dc.l DOS_ERR_NOT_DIRECTORY,.notDirectoryMsg
    dc.l DOS_ERR_NOT_EXECUTABLE,.notExecutableMsg
    dc.l DOS_ERR_NOT_FILE,.notRegularFileMsg
    dc.l DOS_ERR_PATH_NOT_FOUND,.pathNotFoundMsg
    dc.l DOS_ERR_COMMAND_NOT_FOUND,.cmdNotFoundMsg
    dc.l 0,0
.genericErrorMsg:
    dc.b "Code ",0
.errorMsg:
    dc.b "Err: ",0
.notDirectoryMsg:
    dc.b "Not a directory",0
.notRegularFileMsg:
    dc.b "Not a regular file",0
.notExecutableMsg:
    dc.b "Not an executable file",0
.pathNotFoundMsg:
    dc.b "No such file or directory",0
.cmdNotFoundMsg:
    dc.b "Command not found",0
;LineBreakMsg:
    ;dc.b 13,10,0
    even