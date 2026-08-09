    incdir "dos"
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
DosExtErrorCode:rs.b 2
DosSizeof:      rs.b 0

    rsreset
ConsoleClearFunc:           rs.w 3
ConsolePutcFunc:            rs.w 3
ConsolePutsFunc:            rs.w 3
ConsoleGetcFunc:            rs.w 3
ConsoleNormalTextFunc:      rs.w 3
ConsoleBoldTextFunc:        rs.w 3
ConsoleReverseTextFunc:     rs.w 3
ConsoleUnderlinedTextFunc:  rs.w 3
ConsoleCurUpFunc:           rs.w 3
ConsoleCurDnFunc:           rs.w 3
ConsoleCurRtFunc:           rs.w 3
ConsoleCurLtFunc:           rs.w 3
ConsoleFuncSizeof:          rs.b 0
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
OsScratchArea:      rs.b 32
OsDeviceList:       rs.b SD_DEVICE_LIST_SIZE
OsPartitionList:    rs.b PM_PART_LIST_SIZE
OsVolumeList:       rs.b FM_LIST_SIZE
OsDosState:         rs.b DosSizeof
OsConsoleFunc:      rs.b ConsoleFuncSizeof
OsMmcCmdArg:        rs.b 4
OsBootMediaStatus:  rs.w 1
OsAllocatorStart:   rs.l 1
OsSizeof:           rs.b 0

 printt "OsSizeof:"
 printv OsSizeof