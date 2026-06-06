#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char label[16];
    char value[20];
} CpuState;

#define DIS_INSTR_PARTS 16
#define DIS_INSTR_PART_LEN 32
#define DIS_ADDR_LEN 11
#define DIS_BYTES_LEN 100

typedef enum {
    SYM_MNEMONIC,
    SYM_CONSTANT,
    SYM_SYMBOL,
    SYM_REGISTER,
    SYM_UNKNOWN
} InstructionSymbol;

typedef struct {    
    char part[DIS_INSTR_PART_LEN+1];
    InstructionSymbol symbol;
} InstructionPart;

typedef struct {
    InstructionPart parts[DIS_INSTR_PARTS];
    int count;
} Instruction;

typedef struct {
    char address[DIS_ADDR_LEN+1];
    char bytes[DIS_BYTES_LEN+1];
    Instruction instruction;
    int parts;
    bool current;
} Disassembly;

typedef int (*DisassemblyFunc)(void *userdata, Disassembly *disassembly, int maxLines, int *currentLine);

typedef int (*CpuStateFunc)(void *userdata, CpuState *gpRegisters, int maxLines, CpuState *statusRegister);
