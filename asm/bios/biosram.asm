    include hardware.i
    
    rsset SYSTEM_BSS_BASE
__bss_start equ __RS
DetectedRamSize: rs.l 1
SectorBuffer:    rs.b 512
UartData:        rs.l 0
    rsreset
UartRdBuf:       rs.b 256
UartWrPtr:       rs.b 1
UartRdPtr:       rs.b 1

; label: rs.l 1
__bss_end equ __RS    