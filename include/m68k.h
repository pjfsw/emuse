#pragma once

#include <stdint.h>

#include "cpu.h"
#include "readwritefunc.h"

typedef struct {
    uint32_t d[8];
    uint32_t a[7];    
    uint32_t ssp;
    uint32_t usp;
    uint32_t pc;
    uint16_t sr;
} Registers;

typedef struct {
    Cpu cpu;
    Registers registers;
    ReadByteFunc readByteFunc;
    ReadWordFunc readWordFunc;
    void *readWriteUserdata;
} M68k;

void m68kInit(M68k *m68k, ReadByteFunc readByteFunc, ReadWordFunc readWordFunc, void *readWriteUserdata);

int m68kClock(void *userdata);

void m68kReset(void *userdata);