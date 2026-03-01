#pragma once

#include <stdint.h>
#include "ticker.h"
#include "probe.h"

typedef struct {
    DisassemblyFunc disassemblyFunc;
    CpuStateFunc cpuStateFunc;
    void *probeUserdata;
    uint64_t cycle;  
} Cpu;