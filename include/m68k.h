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
} M68kRegisters;

#define SR_FLAGS_S  0x2000
#define SR_FLAGS_I  0x0700
#define SR_FLAGS_C  0x0001
#define SR_FLAGS_V  0x0002
#define SR_FLAGS_Z  0x0004
#define SR_FLAGS_N  0x0008


#define DISASSEMBLY_CACHE_SIZE 64 

typedef struct {
    uint32_t address;
    Disassembly disassembly;
} DisassemblyCache;

typedef struct {
    Cpu cpu;
    M68kRegisters registers;
    ReadByteFunc readByteFunc;
    ReadWordFunc readWordFunc;
    void *readWriteUserdata;
    DisassemblyCache disassemblyCache[DISASSEMBLY_CACHE_SIZE];
    uint32_t firstDisassemblyAddress;
} M68k;

void m68kInit(M68k *m68k, ReadByteFunc readByteFunc, ReadWordFunc readWordFunc, void *readWriteUserdata);

int m68kClock(void *userdata);

void m68kReset(void *userdata);