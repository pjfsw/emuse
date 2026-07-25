    include "storagedevice.i"
    include "fat16.i"
    include "partman.i"
    include "fileman.i"
    include "doslib.i"
    include "dirent.i"

BOOT_LOADER_BASE equ $001000 

DOS_TEMP_AREA_SIZE  equ 16
;____________________________________________________________
;
; DOS State data
;____________________________________________________________

    rsreset
DosCurrentDir:  rs.l 1
DosDirEntry:    rs.b 32
DosPathEntry:   rs.b 16
DosBuffer:      rs.b 512
DosPathContext: rs.b 32
DosTemp:        rs.b DOS_TEMP_AREA_SIZE
DosHunkOffsets: rs.l PROC_MAX_HUNKS
DosSizeof:      rs.b 0

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
OsMmcCmdArg:        rs.b 4
OsMmcStatus:        rs.l 1
OsAllocatorStart:   rs.l 1
OsSizeof:           rs.b 0

 printt "OsSizeof:"
 printv OsSizeof