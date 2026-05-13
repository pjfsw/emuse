#pragma once

#include <stdint.h>

#include "cpu.h"

typedef struct {
    uint32_t d[8];
    uint32_t a[7];    
    uint32_t *sp;
    uint32_t ssp;
    uint32_t usp;
} Registers;

typedef struct {
    Cpu cpu;
    Registers registers;
} M68k;

void m68kInit(M68k *m68k);

int m68kClock(void *userdata);