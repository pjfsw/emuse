#ifndef _IRQ_H
#define _IRQ_H

#include "arch.h"
#include "types.h"

/* Disable all interrupts, returns the previous SR (arch/m68k/irq.asm) */
uint16_t irq_save(void);

/* Restore SR as returned by irq_save (arch/m68k/irq.asm) */
void irq_restore(REG("d0") uint16_t sr);

#endif
