#pragma once

#include <stdint.h>

#include "m68k.h"

typedef enum {
    // Dn
    AM_DREG=0,    
    // An
    AM_AREG=1,    
    // (An)
    AM_ADDRESS=2,
    // (An)+
    AM_ADDRESS_POST_INC=3,
    // -(An)
    AM_ADDRESS_PRE_DEC=4,
    // (d16,An)
    AM_ADDR_DISP=5,
    // (d16,Ax,Dx)
    AM_ADDR_INDEX=6,
    // PC relative (Xn=010), PC relative with index (Xn=011), Absolute short (Xn=000), Absolute long (Xn=001) and Immediate (Xn=100)
    AM_EXT=7
} AddressingMode;

#define AM_EXT_PC_DISP 0x02
#define AM_EXT_PC_INDEX 0x03
#define AM_EXT_ABS_SHORT 0x00
#define AM_EXT_ABS_LONG 0x01
#define AM_EXT_IMMEDIATE 0x04

typedef enum {
    IS_BYTE=0,
    IS_WORD,
    IS_LONG,
} InstructionSize;

typedef enum {
    IF_UNKNOWN=0,
    IF_MOVE,
    IF_BRANCH,
    IF_IMPLIED,
    IF_LEA,
    IF_MOVEM,
    IF_JUMP
} InstructionFamily;

typedef struct {
    AddressingMode mode;
    uint32_t xn;
    uint32_t address;
    uint32_t immediate;
    int16_t displacement;
} EffectiveAddress;

typedef uint32_t (*AluFunc)(uint32_t src, uint32_t dst, uint16_t size, M68kRegisters *regs);

typedef struct DecodedInstruction DecodedInstruction;
typedef int (*ExecFunc)(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata);

struct DecodedInstruction {
    char *mnemonic; 
    uint16_t opcode;
    InstructionFamily family;
    InstructionSize size;
    EffectiveAddress src;
    EffectiveAddress dst;
    int32_t displacement;
    uint16_t condition;
    AluFunc aluFunc;
    ExecFunc execFunc;
};


typedef int (*DecodeFunc)(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata);

// Decode  instruction. Returns number of cycles or 0 if error
// The execution function is stored in execFunc
int decode(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata);

uint32_t align24(uint32_t address);

int getEffectiveAddress(M68kRegisters *registers, uint16_t mode, uint16_t reg, InstructionSize size,
    EffectiveAddress *ea, ReadWordFunc readWordFunc, void *readWriteUserdata);

void increasePc(M68kRegisters *registers);

bool isTargetAddressRegister(DecodedInstruction *di);

void setFlag(M68kRegisters *registers, uint16_t flag, uint16_t value);

bool getFlag(M68kRegisters *registers, uint16_t flag);

void setNZ(M68kRegisters *registers, int32_t value);