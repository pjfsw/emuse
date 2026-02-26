#pragma once

#include <stdint.h>
#include "ticker.h"

typedef struct {
    uint64_t cycle;
    MainTicker mainTicker;
} Cpu;

void cpu_reset(Cpu *cpu);