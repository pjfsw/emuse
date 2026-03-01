#pragma once

#include <stdint.h>

#include "cpu.h"

typedef struct {
    Cpu cpu;
    uint32_t dreg[8];
    uint32_t areg[8];    
} M68k;

void m68kInit(M68k *m68k);

int m68kClock(void *userdata);