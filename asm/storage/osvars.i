    include "storagedevice.i"
    include "fat16.i"
    include "partman.i"
    include "fileman.i"

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
OsVolumeList:       rs.b FM_LIST_SIZE
OsSizeof:       rs.b 0