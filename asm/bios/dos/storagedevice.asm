;____________________________________________________________
;
; Storage abstraction layer 
;  
; Hides details about underlying storage types such as MMC, CF,
; RAM disk etc.

; Allows random access by sector index 
;
; D0-D1/A0-A1 cannot be guaranteed to be preserved on after return.
;
;____________________________________________________________

    include "osvars.i"

SD_ERR_DEVICE_LIST_FULL equ $80010000
SD_ERR_DEVICE_NOT_FOUND equ $80020000
SD_ERR_DEVICE_IO_ERROR  equ $800f0000

;____________________________________________________________
; SDInit
;
; Initialize Storage device module
;____________________________________________________________
SDInit:
    movem.l d7/a6,-(sp)
    bsr.s .sdInitInt
    movem.l (sp)+,d7/a6
    rts
.sdInitInt:    
    lea OSVARS_BASE,a6
    lea OsDeviceList(a6),a1
    moveq #SD_DEVICE_LIST_SIZE/4-1,d7
.loop:    
    clr.l (a1)+
    dbra d7,.loop
    rts

;____________________________________________________________
; SDRegister
;
; Add storage device
;
; A0: pointer to storage device struct
; Return: D0 - device ID > 0 if successful, < 0 if unable to add
;____________________________________________________________
SDRegisterDevice:
    movem.l d7/a6,-(sp)
    bsr.s .sdRegisterInt
    movem.l (sp)+,d7/a6
    rts
.sdRegisterInt:
    lea OSVARS_BASE,a6
    lea OsDeviceList(a6),a1
    moveq #1,d0
    moveq #SD_DEVICE_MAX_COUNT-1,d7
.checkFreeSlot:
    tst.w SD_ID(a1)
    beq.s .foundSlot
    addq.l #1,d0
    lea SD_SIZEOF(a1),a1
    dbra d7,.checkFreeSlot
    move.l #SD_ERR_DEVICE_LIST_FULL,d0
    rts
.foundSlot:    
    moveq #SD_SIZEOF/4-1,d7
.copyDevice:
    move.l (a0)+,(a1)+
    dbra d7,.copyDevice
    rts

;____________________________________________________________
; SDFindDevice
;
; Find storage device by storage id
;
; D0: 2 byte storage id ie. "SD"
; Return: D0 > 0: device handle, D0 < 0: device not found
;____________________________________________________________
SDFindDevice:
    movem.l d7/a6,-(sp)
    bsr.s .sdFindInt
    movem.l (sp)+,d7/a6
    rts
.sdFindInt:    
    lea OSVARS_BASE,a6
    lea OsDeviceList(a6),a1
    moveq #SD_DEVICE_MAX_COUNT-1,d7
    moveq.l #1,d1
.nextDevice:
    cmp.w SD_ID(a1),d0
    beq.s .deviceFound
    addq.l #1,d1
    lea SD_SIZEOF(a1),a1
    dbra d7,.nextDevice
    move.l #SD_ERR_DEVICE_NOT_FOUND,d0
    rts
.deviceFound:
    move.l d1,d0
    rts    

;____________________________________________________________
;
; SDDeviceNumberToStructInA1
; 
; D0 : Device number
; Returns address to device struct in A1 (or 0 if fail)
;____________________________________________________________
SDDeviceNumberToStructInA1:
    subq.l #1,d0
    tst.l d0
    bmi.s .invalidDevice
    cmpi.l #SD_DEVICE_MAX_COUNT,d0
    bcc.s .invalidDevice
    lsl.l #SD_SIZE_SHIFT,d0    
    lea OsDeviceList(a6),a1
    lea (a1,d0.l),a1
    tst.l SD_ID(a1)
    beq.s .invalidDevice
    rts
.invalidDevice:
    move.l #0,a1
    rts

;____________________________________________________________
; SDReadSector
;
; Read a sector from the device
;
; D0: Device number
; D1: Sector number
; A0: Pointer to 512 byte sector buffer
;
; Return: D0 = 0: OK, D0 != 0: Error 
;____________________________________________________________
SDReadSector:
    move.l a6,-(sp)
    bsr.s .sdReadSectorInt
    move.l (sp)+,a6
    rts
.sdReadSectorInt:
    lea OSVARS_BASE,a6
    bsr SDDeviceNumberToStructInA1
    cmp.l #0,a1
    beq .invalidDevice
    move.l SD_READ(a1),a1
    jsr (a1)
    tst.w d0
    bne.s .addErrorCode
    rts
.addErrorCode:
    or.l #SD_ERR_DEVICE_IO_ERROR,d0
    rts
.invalidDevice:
    move.l #SD_ERR_DEVICE_NOT_FOUND,d0
    rts    

;____________________________________________________________
; SDWriteSector
;
; Read a sector from the device
;
; D0: Device handle (as returned by SDFindDevice)
; D1: Sector number
; A0: Pointer to 512 byte sector buffer
;
; Return: D0 = 0: OK, D0 != 0: Error 
;____________________________________________________________
SDWriteSector:
    movem.l d7/a6,-(sp)
    bsr.s .sdWriteSector
    movem.l (sp)+,d7/a6
    rts
.sdWriteSector:
    lea OSVARS_BASE,a6
    bsr SDDeviceNumberToStructInA1
    cmp.l #0,a1
    beq .invalidDevice
    move.l SD_WRITE(a1),a1
    jmp (a1)
.invalidDevice:
    move.l #SD_ERR_DEVICE_NOT_FOUND,d0
    rts    
