#ifndef KERNEL_BIOS_H
#define KERNEL_BIOS_H

#include "types.h"

/* Temporary BIOS console access (arch/m68k/bios.asm) */
void bios_putc(REG("d0") int c);
void bios_puts(REG("a1") const char *s);
int  bios_getc(void);

#endif
