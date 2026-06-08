RAM_SIZE    equ $100000
OREG        equ $d00000
OVR         equ 9
OVR_REG     equ (OREG+OVR)
OVR_ON      equ 0
OVR_OFF     equ $2

SPI_CS      equ 13
SPI_MOSI_CLK equ 15
SPI_CS_OFF  equ 0
SPI_CS_MMC  equ 1
SPI_CS_EXT  equ 2
SPI_CS_ROM  equ 3
IREG        equ $e00001

SYSTEM_BSS_BASE equ $0F8000 ; Allocate 32 KB of variables and stack to the system

    include "uart.i"