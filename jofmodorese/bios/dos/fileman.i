    include "fat16.i"

;____________________________________________________________
;
; FMDeviceList
;____________________________________________________________
    rsreset
FM_DEVICE_NAME  rs.b 4
FM_PM_PART_ID   rs.l 1
FM_FS_DATA      rs.b FAT_SIZEOF
;____________________________________________________________
;
; FM_CREATE_PATH_CTX
;
; Create a path context based on a specific dir identifier
;
; D0: File system context from initialization
; D1: directory first cluster or 0 for root directory
; A0: pointer to target 512 byte sector buffer, must be valid
;     as long as the context is used
; A1: pointer to target 32 byte directory context
;
; Return: D0 = 0: ok
;         D0 ! 0: not ok
;____________________________________________________________
FM_CREATE_PATH_CTX   rs.w 3
;____________________________________________________________
;
; FM_READ_DIR
;
; Read next dir entry from path context
;
; A0: directory context as created by FATOpenDir
; A1: pointer to target 32 byte directory entry
;
; Return: D0 = 0: ok, no more entries (entry is invalid)
;         D0 > 0: ok, end 
;         D0 < 0: not ok
;____________________________________________________________
FM_READ_DIR         rs.w 3

;____________________________________________________________
;
; FATReadFileSector
;
; Read next sector from a file into a buffer
;
; A0: directory context as created by FATCreatePathContext
; A1: 512 byte sector buffer
;
; Return: D0 = 0: end of file reached
;         D0 > 0: number of bytes read
;         D0 < 0: not ok
;____________________________________________________________
FM_READ_FILE_SECTOR rs.w 3
FM_UNPADDED_SIZE    rs.b 0
FM_PADDING          rs.b 64-FM_UNPADDED_SIZE
FM_SIZE             rs.b 0
    if FM_UNPADDED_SIZE > 64
        fail "FM structure exceeds 64 bytes"
    endif
    PRINTV FM_SIZE

FM_MAX_DEVICE_COUNT  equ PM_PART_MAX_COUNT
FM_LIST_SIZE         equ FM_SIZE*FM_MAX_DEVICE_COUNT
