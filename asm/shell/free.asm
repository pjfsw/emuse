    incdir "../lib"
    incdir "../storage"
    include "dirent.i"
    include "errcode.i"

ExecuteFree:
    movem.l a5/a6,-(sp)
    bsr.s .executeFree
    movem.l (sp)+,a5/a6
    rts
.executeFree:
    move.l ROOTLIB_BASE,a6
    lea OSVARS_BASE,a5
    lea .totalMsg(pc),a1
    jsr CONPUTS(a6)

    move.l OsRamSize(a5),d0
    bsr.s .printBytes

    lea .freeMsg(pc),a1
    jsr CONPUTS(a6)

    bsr MemAvail
    bsr.s .printBytes
    moveq #0,d0
    rts
.printBytes: 
    lea DecBuffer,a0
    bsr PrintU32Decimal

    lea DecBuffer,a1
    jsr CONPUTS(a6)

    lea .bytesMsg(pc),a1
    jmp CONPUTS(a6)

.totalMsg:
    dc.b "Total: ",0
.freeMsg:
    dc.b "Free : ",0
.bytesMsg:
    dc.b " bytes",13,10,0
    even