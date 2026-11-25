#include "types.h"
#include "task.h"

static volatile uint32_t jiffies;

/*
 * Called from the timer interrupt (isr.asm). Returns non-zero if
 * the interrupt should switch task.
 */
int timer_tick(void) {
    jiffies++;
    return sched_tick();
}

uint32_t timer_get_jiffies() {
    return jiffies;
}
