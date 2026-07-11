;____________________________________________________________
;
; Read from file
;
; A0: path context as created by DOS_CREATE_CONTEXT
; A1: pointer to target buffer
; D0: number of bytes to read
;
; Return: D0 = 0: end of file
;         D0 > 0: number of bytes read
;         D0 < 0: not ok
;____________________________________________________________
DOS_READ_FILE equ -22
;____________________________________________________________
;
; Read next directory entry
;
; A0: path context as created by DOS_CREATE_CONTEXT
; A1: pointer to target 32 byte directory entry
;
; Return: D0 = 0: ok, no more entries (entry is invalid)
;         D0 > 0: ok, end 
;         D0 < 0: not ok
;____________________________________________________________
DOS_READ_DIR equ -16
;____________________________________________________________
;
; Create context based on path name
;
; A0: Pointer to target path context
; A1: Pointer to path (zero-terminated)
;
; D0: 0 on success, < 0 on error
;____________________________________________________________
DOS_CREATE_CONTEXT equ -10
