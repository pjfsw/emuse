#pragma once

#include <stdint.h>

#include "cpu.h"
#include "readwritefunc.h"

typedef struct {
    uint32_t d[8];
    uint32_t a[8];    
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
#define SR_FLAGS_X  0x0010

#define DISASSEMBLY_CACHE_SIZE 64 

typedef struct {
    uint32_t address;
    Disassembly disassembly;
} DisassemblyCache;

typedef uint8_t (*InterruptLevelFunc)(void *user_data);

// Address error recorded during the current instruction (see m68k.c)
typedef struct {
    bool pending;
    uint32_t address;
    bool read;
} M68kFault;

typedef struct {
    Cpu cpu;
    M68kRegisters registers;
    RwFunc rwFunc;              // Used by instructions: alignment checking wrappers around busRwFunc
    void *readWriteUserdata;    // The M68k itself, for the wrappers
    RwFunc busRwFunc;           // Bus access provided to m68kInit
    void *busUserdata;
    M68kFault fault;
    bool executing;             // Only accesses made while executing can cause an address error
    uint16_t lastOpcode;        // Instruction register for an address error on instruction fetch
    InterruptLevelFunc ilf;
    void *ilfUserdata;
    DisassemblyCache disassemblyCache[DISASSEMBLY_CACHE_SIZE];
    uint32_t firstDisassemblyAddress;
    bool resetState;
    uint8_t interruptLevel;

} M68k;

void m68kInit(M68k *m68k, RwFunc rwFunc, void *readWriteUserdata, InterruptLevelFunc ilf, void *ilfUserdata);

int m68kClock(void *userdata);

void m68kReset(void *userdata);