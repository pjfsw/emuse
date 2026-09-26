PROC_MAX_HUNKS      equ 4

    rsreset
ProcStreamOffset rs.w 1
ProcCurrentHunk: rs.w 1
ProcHunkCount:   rs.w 1
ProcReserved1:   rs.w 1
ProcHunkStart:   rs.l PROC_MAX_HUNKS
ProcHunkSize:    rs.l PROC_MAX_HUNKS
ProcEntry:       rs.l 1
ProcSizeof:      rs.b 0

    rsreset
PM_DEVICE    rs.l 1
PM_INDEX     rs.w 1
PM_TYPE      rs.b 1
PM_FLAGS     rs.b 1
PM_PSTART    rs.l 1
PM_PSIZE     rs.l 1
PM_READ      rs.l 1
PM_WRITE     rs.l 1
PM_NAME      rs.l 1
PM_RESERVED  rs.l 1
PM_SIZEOF    rs.b 0

ROOTLIB_BASE equ $000004
ROOTLIB_VERSION_OFFSET equ -4
ROOTLIB_VERSION equ 1
;____________________________________________________________
;
; DOS_CHANGE_DIR - Change the current working directory 
;
; Input:  A0: pointer to path context 
; Output: D0: 0 upon success, otherwise error code
;____________________________________________________________
DOS_CHANGE_DIR equ -202

;____________________________________________________________
;
; DOS_GET_PART_INFO - Get information about a partition
;
; Input:  D0: partition index
;         A0: pointer to 32 byte structure to store partition
;             information
; Output: D0: 0 = OK
;____________________________________________________________
DOS_GET_PART_INFO equ -196

;____________________________________________________________
;
; DOS_GET_PART_COUNT - Get number of registered partitions
;
; Input:
; Output: D0: The number of registered partitions
;____________________________________________________________
DOS_GET_PART_COUNT equ -190

;____________________________________________________________
;
; DOS_LOAD_EXE - Load executable
;
; Input:  A0: pointer to path context 
; Output: D0: 0 upon success, otherwise error code
;         A0: pointer to Process struct
;____________________________________________________________
DOS_LOAD_PROCESS equ -184

;____________________________________________________________
;
; DOS_READ_FILE - Read from file
;
; Input:  A0: path context as created by DOS_CREATE_CONTEXT
;         A1: pointer to target buffer
;         D0: number of bytes to read
;
; Output: D0 = 0: end of file
;         D0 > 0: number of bytes read
;         D0 < 0: not ok
;____________________________________________________________
DOS_READ_FILE equ -178

;____________________________________________________________
;
; DOS_READ_DIR - Read next directory entry
;
; Input:  A0: path context as created by DOS_CREATE_CONTEXT
;         A1: pointer to target 32 byte directory entry
; Output: D0 = 0: ok, no more entries (entry is invalid)
;         D0 > 0: ok, end 
;         D0 < 0: not ok
;____________________________________________________________
DOS_READ_DIR equ -172

;____________________________________________________________
;
; DOS_CREATE_CONTEXT - Create context based on path name
;
; Input:  A0: Pointer to target path context
;         A1: Pointer to path (zero-terminated)
; Output: D0: 0 on success, < 0 on error
;____________________________________________________________
DOS_CREATE_CONTEXT equ -166

CONCLRLINE  equ -160
CONCLREOL   equ -154
CONSETCRS   equ -148

;____________________________________________________________
;
; CONCRSLEFT - Move cursor left
; Input:  D0: Number of steps to move 0-99 (>99 is undefined)
; Output:
;____________________________________________________________
CONCRSLEFT  equ -142

;____________________________________________________________
;
; CONCRSRIGHT - Move cursor right
; Input:  D0: Number of steps to move 0-99 (>99 is undefined)
; Output:
;____________________________________________________________
CONCRSRIGHT equ -136

;____________________________________________________________
;
; CONCRSUP - Move cursor up
; Input:  D0: Number of steps to move 0-99 (>99 is undefined)
; Output:
;____________________________________________________________
CONCRSUP    equ -130

;____________________________________________________________
;
; CONCRSDOWN - Move cursor down
; Input:  D0: Number of steps to move 0-99 (>99 is undefined)
; Output:
;____________________________________________________________
CONCRSDOWN  equ -124
CONUNDER    equ -118
CONREVERSE  equ -112
CONBOLD     equ -106
CONNORMAL   equ -100
;____________________________________________________________
;
; LIBADD  - Install new or replace existing library
; Input: D0: 32-bit Library identifier
; 
;____________________________________________________________
LIBADD      equ -94

;____________________________________________________________
;
; LIBCLOSE - Close library 
; Input:  D0: 32-bit Library identifier
; Output: D0: 0=OK
;____________________________________________________________
LIBCLOSE    equ -88

;____________________________________________________________
;
; LIBOPEN - Open library 
; Input:  D0: 32-bit Library identifier
;         D1: Version
; Output: D0: Pointer to library base or -1 if unsuccessful
;____________________________________________________________
LIBOPEN     equ -82

;____________________________________________________________
;
; MEMTOTAL - Return the total amount of memory
; Input:  
; Output: D0: Total RAM in bytes
;____________________________________________________________
MEMTOTAL    equ -76

;____________________________________________________________
;
; MEMAVAIL - Return the amount of free memory
; Input:  
; Output: D0: Free RAM in bytes
;____________________________________________________________
MEMAVAIL    equ -70

;____________________________________________________________
;
; MEMFREE - Free allocated memory 
; Input:  A0: Pointer to allocated region
;____________________________________________________________
MEMFREE     equ -64

;____________________________________________________________
;
; MEMALLOC - Allocate memory 
; Input:  D0: Number of bytes to allocate
; Output: D0: pointer to allocated region or 0 if not enough
;            continous RAM
;____________________________________________________________
MEMALLOC    equ -58

;____________________________________________________________
;
; CONGETC - Read character from console
; Input:
; Output: D0: Character 0-255 or -1 if no char available
;____________________________________________________________
CONGETC     equ -52

;____________________________________________________________
; Reserved 
;____________________________________________________________
ROOTRSVD0   equ -46

;____________________________________________________________
;
; CONPUTHEX8 - Write register as 16-bit hex value
; Input:  D0: value to write
; Output:
;____________________________________________________________
CONPUTHEX8  equ -40

;____________________________________________________________
;
; CONPUTHEX16 - Write register as 16-bit hex value
; Input:  D0: value to write
; Output:
;____________________________________________________________
CONPUTHEX16 equ -34

;____________________________________________________________
;
; CONPUTHEX32 - Write register as 32-bit hex value
; Input:  D0: value to write
; Output:
;____________________________________________________________
CONPUTHEX32 equ -28

;____________________________________________________________
;
; CONPUTS - Write null terminated string to console
; Input:  A1: Pointer to null terminated string
; Output:
;____________________________________________________________
CONPUTS     equ -22

;____________________________________________________________
;
; CONPUTC - Write character to console
; Input:  D0: Character 0-255 to write
; Output:
;____________________________________________________________
CONPUTC     equ -16

;____________________________________________________________
;
; CONCLR - Clear console
; Input:
; Output:
;____________________________________________________________
CONCLR      equ -10

