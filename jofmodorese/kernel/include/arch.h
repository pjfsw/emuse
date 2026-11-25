#ifndef KERNEL_ARCH_H
#define KERNEL_ARCH_H

/*
 * Compiler/architecture specifics.
 *
 * REG(r) passes a function argument in a specific register (vbcc),
 * e.g. void bios_putc(REG("d0") int c). It expands to nothing for
 * other compilers, e.g. IntelliSense or host builds.
 */
#ifdef __VBCC__
#define REG(r) __reg(r)
#else
#define REG(r)
#endif

#endif
