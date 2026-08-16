;
; PSB - Process startup block
; 
; Used to pass shell parameters to a process

PSB_ARG_LENGTH equ 128

    rsreset
; Argument part of command line launch, not including 
; redirection or shell specific switches. Null-terminated
PsbArg    rs.b PSB_ARG_LENGTH
; Pointer to environment variable list 
PsbEnvPtr rs.l 1
PsbSizeOf rs.b 0
