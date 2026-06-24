IRQV_SPI    equ $000064 ; Level 1
IRQV_LVL2   equ $000068 ; Level 2
IRQV_TIMER  equ $00006c ; Level 3
IRQV_BLANK  equ $000070 ; Level 4
IRQV_UART   equ $000074 ; Level 5
IRQV_LVL6   equ $000078 ; Level 2
IRQV_LVL7   equ $00007c ; Level 2

EXEC_BASE   equ $4
RAM_LIMIT   equ $800000
OREG        equ $d00000
OVR         equ 9
OVR_REG     equ (OREG+OVR)
OVR_ON      equ 0
OVR_OFF     equ $2

TIMER_ACT   equ 5
TIMER_FREQ  equ 7
TIMER_75HZ  equ 1
TIMER_150HZ equ 2
TIMER_300HZ equ 3
SPI_CS      equ 13
SPI_MOSI_CLK equ 15
SPI_CS_OFF  equ 0
SPI_CS_MMC  equ 1
SPI_CS_EXT  equ 2
SPI_CS_ROM  equ 3
TIMERACK    equ $a00001
IREG        equ $e00001

SYSTEM_BSS_BASE equ $000400 
ALLOCATOR_BASE  equ $001000 
SIMPLE_LOADER_BASE equ $010000 

    include "uart.i"