    incdir "../include"
    include psb.i

MAX_CMDLINE_LENGTH equ PSB_ARG_LENGTH 
    
    rsreset
ShellInputBuffer:   rs.b MAX_CMDLINE_LENGTH 
ShellSizeof:        rs.b 0