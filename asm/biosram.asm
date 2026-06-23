    include hardware.i
    
    rsset SYSTEM_BSS_BASE
__bss_start equ __RS
detectedRamSize: rs.l 1

; label: rs.l 1
__bss_end equ __RS    