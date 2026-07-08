    rsreset
SD_ID       rs.w 1
SD_READ     rs.l 1
SD_WRITE    rs.l 1
SD_RESERVED rs.w 11
SD_SIZEOF   rs.b 0

SD_SIZE_SHIFT       equ 5   ; 32 bytes
SD_DEVICE_MAX_COUNT equ 4
SD_DEVICE_LIST_SIZE equ SD_SIZEOF*SD_DEVICE_MAX_COUNT

;____________________________________________________________
;
; Operating system variables
;____________________________________________________________
OSVARS_BASE equ SYSTEM_BSS_BASE

    rsreset
OsRamSize:          rs.l 1
OsSectorBuffer:     rs.b 512
OsUartRdBuf:        rs.b 256
OsUartWrPtr:        rs.b 1
OsUartRdPtr:        rs.b 1
OsDeviceList:       rs.b SD_DEVICE_LIST_SIZE

OsSizeof:       rs.b 0