#ifndef KERNEL_BIOS_H
#define KERNEL_BIOS_H

#include "arch.h"
#include "types.h"

/* Temporary BIOS console access (arch/m68k/bios.asm) */
void bios_putc(REG("d0") int c);
void bios_puts(REG("a1") const char *s);
int  bios_getc(void);
void bios_put_hex32(REG("d0") uint32_t value);

#endif
