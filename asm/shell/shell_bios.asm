    incdir ..
    incdir ../storage

;
; Herein lies code that eventually should be moved from the shell to BIOS
;
ShellBiosInit:
    bsr InstallFakeLibraries
    bsr InstallExceptionHandlers

    bsr MemInit
    bsr InitDosVars
    bsr InitStorageDevices    

    move.l ROOTLIB_BASE,a6 
        
    lea DosLoadingMsg,a1
    jsr CONPUTS(a6)
    
    tst.l d0
    beq.s .storageOk
    move.l d0,d7
    lea InitStorageErrorMsg(pc),a1
    jsr CONPUTS(a6)
    move.l d7,d0
    bsr PrintErrorCode
    rts
.storageOk:
    rts

InitStorageDevices:
    bsr FMInit    
    bsr MMCInit  
    move.w d0,MmcStatus
    beq.s .mmcOk
    and.w #$ffff,d0
    rts
.mmcOk:
    lea MmcStorageDevice,a0
    bsr FMRegisterDevice
    rts
    
InstallExceptionHandlers:
    lea ExceptionAddressError,a0
    move.l a0,$0000000C
    lea ExceptionIllegalInstruction,a0
    move.l a0,$00000010    
    rts

InitDosVars:
    lea OSVARS_BASE,a0
    lea OsDosState(a0),a0
    clr.l DosCurrentDir(a0)
    move.b #'/',DosCurDirName(a0)
    clr.b 1+DosCurDirName(a0)
    rts

InstallFakeLibraries:
    move.l ROOTLIB_BASE,a5
    lea -8(a5),a5
    lea JT_ROOT_LIB_BASE-8,a6
    moveq #7,d7
.copyVectors:
    move.l (a5),(a6)
    lea -6(a5),a5
    lea -6(a6),a6
    dbra d7,.copyVectors
    move.l #JT_ROOT_LIB_BASE,ROOTLIB_BASE
    move.l #JT_DOS_LIB_BASE,DosLibBase   
    rts

JT_DOS_GET_PROC_STATE:  jmp FMGetProcDosState
JT_DOS_GET_PART_INFO:   jmp PMGetPartitionInfo
JT_DOS_GET_PART_COUNT:  jmp PMGetPartitionCount
JT_DOS_LOAD_EXE:        jmp FMLoadExecutable
JT_DOS_READ_FILE:       jmp FMReadFile
JT_DOS_READ_DIR:        jmp FMReadDir
JT_DOS_CREATE_CTX:      jmp FMCreateContext
JT_DOS_VERSION:         dc.l 1
JT_DOS_LIB_BASE:    

ConPuts:
ConPutc:
ConPutHex32:
ConPutHex16:
ConPutHex8:
ConGetChar:
ConClr:
    include jt_root.asm
JT_ROOT_LIB_BASE:

    include exceptions.asm
    include mmc.asm
    include storagedevice.asm
    include partman.asm
    include fat16.asm
    include fileman.asm
    include exeloader.asm
    include memman.asm

MmcStorageDevice:
    dc.b "SD"
    dc.l MMCReadSector
    dc.l MMCWriteSector
    blk.w 10,0
