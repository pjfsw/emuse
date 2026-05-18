#include <string.h>
#include <stdio.h>
#include "m68k.h"
#include "decode.h"

static inline void setUser(M68k *m68k) {
    m68k->registers.sr = m68k->registers.sr & (0xffff-SR_FLAGS_S);    
}

static inline void setSupervisor(M68k *m68k) {
    m68k->registers.sr = m68k->registers.sr | SR_FLAGS_S;    
}

static bool isSupervisor(M68k *m68k) {
    return (m68k->registers.sr & SR_FLAGS_S) != 0;
}

static char* writeAddress(char *target, uint32_t address) {
    sprintf(target, "%06x", address & 0xffffff);
    return target;
}

static void addDisassembly(Instruction *instruction, char *string, InstructionSymbol type) {
    strncpy(instruction->parts[instruction->count].part, string, DIS_INSTR_PART_LEN);
    instruction->parts[instruction->count].symbol = type;
    instruction->count++;
}

static void addPadding(Instruction *instruction) {
    int n = 0;
    for (int i = 0; i < instruction->count; i++) {
        n += strlen(instruction->parts[i].part);
    }   
    char s[100];
    sprintf(s, "%-*s", 7-n, "");
    addDisassembly(instruction, s, SYM_SYMBOL);
}

static void disassembleEa(Instruction *instruction, EffectiveAddress *ea, InstructionSize size) {
    char s[100];
    s[0] = 0;
    if (ea->mode == AM_DREG) {
        sprintf(s, "D%d", ea->xn);
        addDisassembly(instruction, s, SYM_REGISTER);
    } else if (ea->mode == AM_AREG) {
        sprintf(s, "A%d", ea->xn);
        addDisassembly(instruction, s, SYM_REGISTER);
    } else if (ea->mode == AM_EXT) {
        if (ea->xn == AM_EXT_IMMEDIATE) {
            addDisassembly(instruction, "#", SYM_SYMBOL);            
            if (size == IS_BYTE) {
                sprintf(s, "$%X", (uint8_t)ea->immediate);
            } else if (size == IS_WORD) {
                sprintf(s, "$%X", (uint16_t)ea->immediate);
            } else if (size == IS_LONG) {
                sprintf(s, "$%X", ea->immediate);
            }
            addDisassembly(instruction, s, SYM_CONSTANT);            
        }
    }
}

static void disassembleMove(DecodedInstruction *di, Instruction *instruction) {
    switch (di->size) {
        case IS_BYTE:
            addDisassembly(instruction, ".B", SYM_MNEMONIC);
            break;
        case IS_LONG:
            addDisassembly(instruction, ".L", SYM_MNEMONIC);
            break;
        case IS_WORD:
            addDisassembly(instruction, ".W", SYM_MNEMONIC);
            break;
    }
    addPadding(instruction);
    addDisassembly(instruction, " ", SYM_SYMBOL);
    disassembleEa(instruction, &di->src, di->size);
    addDisassembly(instruction, ",", SYM_SYMBOL);
    disassembleEa(instruction, &di->dst, di->size);
}

static void disassembleBranch(DecodedInstruction *di, uint32_t pc, Instruction *instruction) {
    if (di->size == IS_BYTE) {
        addDisassembly(instruction, ".S", SYM_MNEMONIC);
    }
    addPadding(instruction);
    char s[100];
    sprintf(s, " $%06X", pc + (int32_t)di->displacement);
    addDisassembly(instruction, s, SYM_CONSTANT);
}


static void disassemble(M68k *cpu, M68kRegisters *regs, char *address, Instruction *instruction) {
    DecodedInstruction di;
    memset(&di, 0, sizeof(DecodedInstruction));
    writeAddress(address, regs->pc);
    ExecFunc dummyExec;
    uint16_t opcode = cpu->readWordFunc(cpu->readWriteUserdata, regs->pc);
    decode(&di, regs, cpu->readByteFunc, cpu->readWordFunc, cpu->readWriteUserdata, &dummyExec);
    instruction->count = 0;
    char s[100];
    if (di.family == IF_UNKNOWN) {
        addDisassembly(instruction, di.mnemonic, SYM_UNKNOWN);
    } else {
        addDisassembly(instruction, di.mnemonic, SYM_MNEMONIC);
    }
    switch (di.family) {
        case IF_MOVE:
            disassembleMove(&di, instruction);
            break;
        case IF_BRANCH:
            disassembleBranch(&di, regs->pc, instruction);
            break;
        case IF_UNKNOWN:
            char s[100];
            sprintf(s, " $%04x", opcode);
            addDisassembly(instruction, s, SYM_UNKNOWN);
            break;
    }
}

static int getDisassembly(void *userdata, Disassembly *disassembly, int maxLines, int *currentLine) {
    M68k *cpu = (M68k *)userdata;

    M68kRegisters regs = cpu->registers;
    DisassemblyCache *cache = cpu->disassemblyCache;
    int firstLine = -1;
    *currentLine = -1;
    uint32_t lastDisassemblyAddress = 0;
    for (int i = 0; i < DISASSEMBLY_CACHE_SIZE; i++) {
        lastDisassemblyAddress = cache[i].address;
        if (cache[i].address == cpu->firstDisassemblyAddress) {
            firstLine = i;
        }
        if (cache[i].address == regs.pc) {
            *currentLine = i;
        }
    }    
    if ((firstLine >= 0) && (*currentLine >= 0)) {
        const int pad = 3;
        if ((*currentLine-firstLine) >= (maxLines-pad)) {
            int n = maxLines-pad-(*currentLine-firstLine);
            if (n < 1) {
                n = 1;
            }
            firstLine += n;
            //printf("Change first line %02d=$%04x\n", firstLine, cache[firstLine].address);
            cpu->firstDisassemblyAddress = cache[firstLine].address;
            *currentLine -= n;
        }
        regs.pc = cpu->firstDisassemblyAddress;
    } else {
        // Default, just reset everything
        firstLine = 0;  
        *currentLine = 0;              
        cpu->firstDisassemblyAddress = regs.pc;
    }
    // Just disassemble everything again because of self modifying code
    for (int i = 0; i < DISASSEMBLY_CACHE_SIZE; i++) {
        cache[i].address = regs.pc;
        disassemble(cpu, &regs, cache[i].disassembly.address, &cache[i].disassembly.instruction);
    }

    //printf("First line %d, Current Line %d, First Address %06x\n", firstLine, *currentLine, cache[0].address);
    for (int i = 0; i < maxLines; i++) {
        memcpy(&disassembly[i], &cache[i + firstLine].disassembly, sizeof(Disassembly));
    }
    return maxLines;
}

static void writeU32(char *dest, uint32_t value) {
    sprintf(dest, "$%08X", value);
}

static char srFlags[] = "TTSM0III000XNZVC";

static void writeSR(char *dest, M68k *cpu) {
    uint16_t sr = cpu->registers.sr;
    int n = 0;
    for (int i = 0; i < 16; i++) {
        if (sr & 0x8000) {
            dest[n] = srFlags[i];
        } else {
            dest[n] = '.';
        }
        if (i == 7) {
            n++;
            dest[n] = ' ';
        }
        n++;
        sr = sr << 1;
    }
    dest[16] = 0;
}

static inline uint32_t *getSP(M68k *cpu) {
    return isSupervisor(cpu) ? &cpu->registers.ssp : &cpu->registers.usp;
}

static int getCpuState(void *userdata, CpuState *gpRegisters, int maxLines, CpuState *statusRegister) {
    M68k *cpu = (M68k *)userdata;

    int actualLines = 0;
    for (int i = 0; i < 8; i++) {
        if (actualLines < maxLines) {
            gpRegisters[i].label[0] = 'D';
            gpRegisters[i].label[1] = 48+i;
            gpRegisters[i].label[2] = 0;
            writeU32(gpRegisters[i].value, cpu->registers.d[i]);
            actualLines++;
        }
    }
    for (int i = 0; i < 7; i++) {
        if (actualLines < maxLines) {
            gpRegisters[i+8].label[0] = 'A';
            gpRegisters[i+8].label[1] = 48+i;
            gpRegisters[i+8].label[2] = 0;
            writeU32(gpRegisters[i+8].value, cpu->registers.a[i]);
            actualLines++;
        }
    }
    if (actualLines < maxLines) {
        gpRegisters[15].label[0] = 'S';
        gpRegisters[15].label[1] = 'P';
        gpRegisters[15].label[2] = 0;
        writeU32(gpRegisters[15].value, *getSP(cpu));
        actualLines++;
    }
    if (actualLines < maxLines) {
        statusRegister->label[0] = 'S';
        statusRegister->label[1] = 'R';
        statusRegister->label[2] = 0;
        writeSR(statusRegister->value, cpu);
    }
    return actualLines;
}


int m68kClock(void *userdata) {
    M68k *cpu = (M68k *)userdata;    
    DecodedInstruction di;
    ExecFunc execFunc;
    int cycles = decode(&di, &cpu->registers, cpu->readByteFunc, cpu->readWordFunc, cpu->readWriteUserdata, &execFunc);
    if (execFunc != NULL) {
        cycles += execFunc(&di, &cpu->registers, cpu->readByteFunc, cpu->readWordFunc, cpu->readWriteUserdata);
    }
    return cycles;
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