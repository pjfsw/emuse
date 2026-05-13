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

static void writeU32(char *dest, uint32_t value) {
    sprintf(dest, "$%08x", value);
}

static int getCpuState(void *userdata, CpuState *cpuState, int maxLines) {
    M68k *cpu = (M68k *)userdata;

    int actualLines = 0;
    for (int i = 0; i < 8; i++) {
        if (actualLines < maxLines) {
            cpuState[i].label[0] = 'D';
            cpuState[i].label[1] = 48+i;
            cpuState[i].label[2] = 0;
            writeU32(cpuState[i].value, cpu->registers.d[i]);
            actualLines++;
        }
    }
    for (int i = 0; i < 7; i++) {
        if (actualLines < maxLines) {
            cpuState[i+8].label[0] = 'A';
            cpuState[i+8].label[1] = 48+i;
            cpuState[i+8].label[2] = 0;
            writeU32(cpuState[i+8].value, cpu->registers.a[i]);
            actualLines++;
        }
    }
    return actualLines;
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