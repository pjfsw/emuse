    include "storagedevice.i"
    include "fat16.i"
    include "partman.i"
    include "fileman.i"
    include "doslib.i"

DOS_TEMP_AREA_SIZE  equ 16
;____________________________________________________________
;
; DOS State data
;____________________________________________________________

    rsreset
DosCurrentDir:  rs.l 1
DosDirEntry     rs.b 32
DosPathEntry    rs.b 16
DosBuffer       rs.b 512
DosTemp:        rs.b DOS_TEMP_AREA_SIZE
DosSizeof       rs.b 0

;____________________________________________________________
;
; Operating system variables
;____________________________________________________________
OSVARS_BASE equ SYSTEM_BSS_BASE

OS_MAX_PROCESS_COUNT equ 16

    rsreset
OsRamSize:          rs.l 1
OsSectorBuffer:     rs.b 512
OsUartRdBuf:        rs.b 256
OsUartWrPtr:        rs.b 1
OsUartRdPtr:        rs.b 1
OsDeviceList:       rs.b SD_DEVICE_LIST_SIZE
OsPartitionList:    rs.b PM_PART_LIST_SIZE
OsVolumeList:       rs.b FM_LIST_SIZE
OsDosState:         rs.b DosSizeof
OsSizeof:           rs.b 0
