DOSInit:
    bsr InitDosVars
    bsr InitStorageDevices    

    move.l ROOTLIB_BASE,a6 
            
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
    lea OSVARS_BASE,a0
    move.w d0,OsMmcStatus(a0)
    beq.s .mmcOk
    and.w #$ffff,d0
    rts
.mmcOk:
    lea MmcStorageDevice,a0
    bsr FMRegisterDevice
    rts
    
InitDosVars:
    lea OSVARS_BASE,a0
    lea OsDosState(a0),a0
    clr.l DosCurrentDir(a0)
    rts

PrintErrorCode:
    jsr CONPUTHEX32(a6)
    lea LineBreakMsg(pc),a1
    jmp CONPUTS(a6)

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
InitStorageErrorMsg:
    dc.b 13,10,"Failed to initialize boot device: ",0
    even
