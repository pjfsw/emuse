/*
 * vlink linker script for the BIOS ROM image.
 *
 * The ROM is mapped at $F00000. The vector table (bios.asm "org $f00000")
 * and the jump table/code (bios.asm "org $f00400") are placed at fixed
 * addresses.
 *
 * Writable data (.data/.bss) is not supported in ROM. Such sections are
 * routed to a zero-sized memory region so that the link fails if any
 * object contributes writable data.
 */
MEMORY
{
    rom    : org = 0xF00000, len = 0x10000
    nodata : org = 0x000000, len = 0
}

SECTIONS
{
    .vectors 0xF00000 : { *(org0001*) } > rom
    .text    0xF00400 : { *(org0002*) *(CODE) *(text) *(.text) *(.rodata) } > rom
    .data             : { *(DATA) *(data) *(.data) *(BSS) *(bss) *(.bss) } > nodata
}
