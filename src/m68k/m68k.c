#include <string.h>
#include "m68k.h"
#include <stdio.h>

static int getDisassembly(void *userdata, Disassembly *disassembly, int maxLines) {
    if (maxLines > 0) {
        strncpy(disassembly[0].instruction, "moveq #0,d0", DIS_INSTR_LEN);
        strncpy(disassembly[0].address,  "$0010 0000", DIS_ADDR_LEN);
        disassembly[1].current = false;
        return 1;
    }
    return 0;
}

static int getCpuState(void *userdata, CpuState *cpuState, int maxLines) {
    if (maxLines > 0) {
        strcpy(cpuState[0].label, "D0");
        strcpy(cpuState[0].value, "$12345678");
        return 1;
    }
    return 0;
}

int m68kClock(void *userdata) {
    M68k *cpu = (M68k *)userdata;
    return 4;
}


void m68kInit(M68k *m68k) {
    memset(m68k, 0, sizeof(M68k));
    m68k->cpu.cpuStateFunc = getCpuState;
    m68k->cpu.disassemblyFunc = getDisassembly;
    m68k->cpu.probeUserdata = m68k;
}