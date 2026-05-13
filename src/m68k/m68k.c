#include <string.h>
#include "m68k.h"
#include <stdio.h>

#define SR_T  0x8000
#define SR_S  0x2000
#define SR_I  0x0700

static inline void setUser(M68k *m68k) {
    m68k->registers.sr = m68k->registers.sr & (0xffff-SR_S);    
}

static inline void setSupervisor(M68k *m68k) {
    m68k->registers.sr = m68k->registers.sr | SR_S;    
}

static bool isSupervisor(M68k *m68k) {
    return (m68k->registers.sr & SR_S) != 0;
}

static char* writeAddress(char *target, uint32_t address) {
    sprintf(target, "$%06x", address & 0xffffff);
    return target;
}

static int getDisassembly(void *userdata, Disassembly *disassembly, int maxLines) {
    M68k *cpu = (M68k *)userdata;

    if (maxLines > 0) {
        strncpy(disassembly[0].instruction, "moveq #0,d0", DIS_INSTR_LEN);
        writeAddress(disassembly[0].address, cpu->registers.pc);
        disassembly[1].current = false;
        return 1;
    }
    return 0;
}

static void writeU32(char *dest, uint32_t value) {
    sprintf(dest, "$%08x", value);
}

static char srFlags[] = "TTSM0III000XNZVC";

static void writeSR(char *dest, M68k *cpu) {
    uint16_t sr = cpu->registers.sr;
    for (int i = 0; i < 16; i++) {
        if (sr & 0x8000) {
            dest[i] = srFlags[i];
        } else {
            dest[i] = '.';
        }
        sr = sr << 1;
    }
    dest[16] = 0;
}

static inline uint32_t *getSP(M68k *cpu) {
    return isSupervisor(cpu) ? &cpu->registers.ssp : &cpu->registers.usp;
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
    if (actualLines < maxLines) {
        cpuState[15].label[0] = 'S';
        cpuState[15].label[1] = 'P';
        cpuState[15].label[2] = 0;
        writeU32(cpuState[15].value, *getSP(cpu));
        actualLines++;
    }
    if (actualLines < maxLines) {
        cpuState[16].label[0] = 'S';
        cpuState[16].label[1] = 'R';
        cpuState[16].label[2] = 0;
        writeSR(cpuState[16].value, cpu);
        actualLines++;
    }
    return actualLines;
}

int m68kClock(void *userdata) {
    M68k *cpu = (M68k *)userdata;
    return 4;
}

void m68kInit(M68k *m68k, ReadByteFunc readByteFunc, ReadWordFunc readWordFunc, void *readWriteUserdata) {
    memset(m68k, 0, sizeof(M68k));
    m68k->cpu.cpuStateFunc = getCpuState;
    m68k->cpu.disassemblyFunc = getDisassembly;
    m68k->cpu.probeUserdata = m68k;
    m68k->readByteFunc = readByteFunc;
    m68k->readWordFunc = readWordFunc;
    m68k->readWriteUserdata = readWriteUserdata;
}

static uint16_t readWord(M68k *m68k, uint32_t address) {
    return m68k->readWordFunc(m68k->readWriteUserdata, address);
}

static uint32_t readLong(M68k *m68k, uint32_t address) {
    return (((uint32_t)readWord(m68k, address)) << 16) | (uint32_t)readWord(m68k, address + 2);
}

void m68kReset(void *userdata) {
    M68k *m68k = (M68k*)userdata;
    m68k->registers.ssp = readLong(m68k, 0);
    m68k->registers.pc = readLong(m68k, 4);
    setSupervisor(m68k);
}