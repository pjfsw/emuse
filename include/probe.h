#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char label[16];
    char value[16];
} CpuState;

#define DIS_INSTR_LEN 32
#define DIS_ADDR_LEN 11
typedef struct {
    char address[DIS_ADDR_LEN+1];
    char instruction[DIS_INSTR_LEN+1];
    bool current;
} Disassembly;

typedef int (*DisassemblyFunc)(void *userdata, Disassembly *disassembly, int maxLines);

typedef int (*CpuStateFunc)(void *userdata, CpuState *cpuState, int maxLines);

