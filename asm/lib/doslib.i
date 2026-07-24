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


;____________________________________________________________
; DOS_GET_PROC_STATE - Get current process DOS state
;
; Input: 
; Output: Pointer to the current state in A0
;____________________________________________________________
DOS_GET_PROC_STATE  equ -46

;____________________________________________________________
;
; DOS_GET_PART_INFO - Get information about a partition
;
; Input:  D0: partition index
;         A0: pointer to 32 byte structure to store partition
;             information
; Output: D0: 0 = OK
;____________________________________________________________
DOS_GET_PART_INFO equ -40

;____________________________________________________________
;
; DOS_GET_PART_COUNT - Get number of registered partitions
;
; Input:
; Output: D0: The number of registered partitions
;____________________________________________________________
DOS_GET_PART_COUNT equ -34

;____________________________________________________________
;
; DOS_LOAD_EXE - Load executable
;
; Input:  A0: pointer to path context 
; Output: D0: 0 upon success, otherwise error code
;         A0: pointer to first code entry
;____________________________________________________________
DOS_LOAD_EXE equ -28

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
DOS_READ_FILE equ -22

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
DOS_READ_DIR equ -16

;____________________________________________________________
;
; DOS_CREATE_CONTEXT - Create context based on path name
;
; Input:  A0: Pointer to target path context
;         A1: Pointer to path (zero-terminated)
; Output: D0: 0 on success, < 0 on error
;____________________________________________________________
DOS_CREATE_CONTEXT equ -10
