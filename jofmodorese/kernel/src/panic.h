#ifndef PANIC_H
#define PANIC_H

#include "types.h"

/*
 * State of an exception, filled in by the exception handlers in
 * isr.asm before kpanic is called.
 */
extern const char *panic_msg;       /* What happened */
extern uint32_t    panic_regs[15];  /* d0-d7/a0-a6 at the exception */
extern uint16_t   *panic_group0;    /* Bus/address error frame, or NULL */
extern uint16_t   *panic_frame;     /* SR (word) and PC (long) */

/* Print the exception state and halt, does not return */
void kpanic(void);

#endif
