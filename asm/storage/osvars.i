    include "storagedevice.i"
    include "fat16.i"
    include "partman.i"
    include "fileman.i"

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
OsCurrentProcess:   rs.l 1
OsProcesses:        rs.l OS_MAX_PROCESS_COUNT
OsSizeof:           rs.b 0

;____________________________________________________________
;
; DOS State data
;____________________________________________________________
    rsreset
DOSINFO_DIRENT  rs.b 32
DOSINFO_PATHENT rs.b 16
DOSINFO_BUFFER  rs.b 512
DOSINFO_SIZEOF  rs.b 0

;____________________________________________________________
;
; Process data
;____________________________________________________________
    rsreset
ProcId:         rs.l 1
ProcDosState:   rs.b DOSINFO_SIZEOF
ProcUSizeOf:    rs.b 0
ProcPadding:    rs.b 1008-ProcUSizeOf
ProcSizeOf:     rs.b 0
    if ProcUSizeOf > 1008
        fail "Process structure exceeds 1 KB bytes"
    endif
    PRINTV ProcSizeOf
