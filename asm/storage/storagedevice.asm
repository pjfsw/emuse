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
; Accessed with a 32 byte storage struct
; 
; uint32_t ID       - Identifier of the device including device number
; uint32_t INIT     - Init function. Takes the ID as input in D0 and returns result in D0.
; uint32_t 
;____________________________________________________________

    rsreset
SD_ID       rs.l 1
SD_READ     rs.l 1
SD_WRITE    rs.l 1
SD_RESERVED rs.l 5
SD_SIZE     rs.b 0

SD_SIZE_SHIFT       equ 5   ; 32 bytes
SD_DEVICE_LIST_SIZE equ 256
SD_DEVICE_LIMIT     equ 8

SD_ERR_DEVICE_LIST_FULL equ $80010000
SD_ERR_DEVICE_NOT_FOUND equ $80020000
SD_ERR_DEVICE_IO_ERROR  equ $800f0000


SDInit:
    move.l d7,-(sp)
    bsr.s .sdInitInt
    move.l (sp)+,d7
    rts
.sdInitInt:    
    lea SDDeviceList,a1
    moveq #SD_DEVICE_LIST_SIZE/4-1,d7
    moveq #0,d0
.loop:    
    move.l d0,(a1)+
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
    move.l d7,-(sp)
    bsr.s .sdRegisterInt
    move.l (sp)+,d7
    rts
.sdRegisterInt:
    lea SDDeviceList,a1
    moveq #1,d0
    moveq #SD_DEVICE_LIMIT-1,d7
.checkFreeSlot:
    tst.l SD_ID(a1)
    beq.s .foundSlot
    addq.l #1,d0
    lea SD_SIZE(a1),a1
    dbra d7,.checkFreeSlot
    move.l #SD_ERR_DEVICE_LIST_FULL,d0
    rts
.foundSlot:    
    moveq #SD_SIZE/4-1,d7
.copyDevice:
    move.l (a0)+,(a1)+
    dbra d7,.copyDevice
    rts

;____________________________________________________________
; SDFindDevice
;
; Find storage device by storage id
;
; D0: 4 byte storage id ie. "mmc0"
; Return: D0 > 0: device handle, D0 < 0: device not found
;____________________________________________________________
SDFindDevice:
    move.l d7,-(sp)
    bsr.s .sdFindInt
    move.l (sp)+,d7
    rts
.sdFindInt:    
    lea SDDeviceList,a1
    moveq #SD_DEVICE_LIMIT-1,d7
    moveq.l #1,d1
.nextDevice:
    cmp.l SD_ID(a1),d0
    beq.s .deviceFound
    addq.l #1,d1
    lea SD_SIZE(a1),a1
    dbra d7,.nextDevice
    move.l #SD_ERR_DEVICE_NOT_FOUND,d0
    rts
.deviceFound:
    move.l d1,d0
    rts    

SDDeviceNumberToStructInA1:
    subq.l #1,d0
    tst.l d0
    bmi.s .invalidDevice
    cmpi.l #SD_DEVICE_LIMIT,d0
    bcc.s .invalidDevice
    lsl.l #SD_SIZE_SHIFT,d0 
    lea SDDeviceList,a1
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
; D0: Device handle (as returned by SDFindDevice)
; D1: Sector number
; A0: Pointer to 512 byte sector buffer
;
; Return: D0 = 0: OK, D0 != 0: Error 
;____________________________________________________________
SDReadSector:
    move.l d7,-(sp)
    bsr.s .sdReadSector
    move.l (sp)+,d7
    rts
.sdReadSector:
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
    move.l d7,-(sp)
    bsr.s .sdWriteSector
    move.l (sp)+,d7
    rts
.sdWriteSector:
    bsr SDDeviceNumberToStructInA1
    cmp.l #0,a1
    beq .invalidDevice
    move.l SD_WRITE(a1),a1
    jmp (a1)
.invalidDevice:
    move.l #SD_ERR_DEVICE_NOT_FOUND,d0
    rts    
