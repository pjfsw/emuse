    rsreset
SD_ID       rs.w 1
SD_READ     rs.l 1
SD_WRITE    rs.l 1
SD_RESERVED rs.w 11
SD_SIZEOF   rs.b 0

SD_SIZE_SHIFT       equ 5   ; 32 bytes
SD_DEVICE_MAX_COUNT equ 4
SD_DEVICE_LIST_SIZE equ SD_SIZEOF*SD_DEVICE_MAX_COUNT

    rsreset
PM_DEVICE    rs.l 1
PM_INDEX     rs.w 1
PM_TYPE      rs.b 1
PM_FLAGS     rs.b 1
PM_PSTART    rs.l 1
PM_PSIZE     rs.l 1
PM_READ      rs.l 1
PM_WRITE     rs.l 1
PM_RESERVED  rs.l 2
PM_SIZEOF    rs.b 0

PM_SIZE_SHIFT       equ 5   ; 32 bytes
PM_PART_MAX_COUNT   equ 4
PM_PART_LIST_SIZE   equ PM_SIZEOF*PM_PART_MAX_COUNT


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
OsPartitionList:    rs.b PM_PART_LIST_SIZE
OsSizeof:       rs.b 0