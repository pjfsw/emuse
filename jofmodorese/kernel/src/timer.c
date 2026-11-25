#include "types.h"

static volatile uint32_t jiffies;

void timer_tick(void) {
    jiffies++;
}

uint32_t timer_get_jiffies() {
    return jiffies;
}
