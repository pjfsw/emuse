;____________________________________________________________
;
; Jump table for the DOS library
;____________________________________________________________
JT_DOS_CHANGE_DIR:      jmp FMChangeDirectory
JT_DOS_GET_PART_INFO:   jmp PMGetPartitionInfo
JT_DOS_GET_PART_COUNT:  jmp PMGetPartitionCount
JT_DOS_LOAD_EXE:        jmp FMLoadExecutable
JT_DOS_READ_FILE:       jmp FMReadFile
JT_DOS_READ_DIR:        jmp FMReadDir
JT_DOS_CREATE_CTX:      jmp FMCreateContext
JT_DOS_VERSION:         dc.l 1
JT_DOS_LIB_BASE:    
