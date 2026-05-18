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
    IS_FIRST=0,
    IS_BYTE,
    IS_LONG,
    IS_WORD
} InstructionSize;

typedef enum {
    IF_UNKNOWN=0,
    IF_MOVE,
    IF_BRANCH
} InstructionFamily;

typedef struct {
    AddressingMode mode;
    uint32_t xn;
    uint32_t address;
    uint32_t immediate;
} EffectiveAddress;

typedef struct {
    char *mnemonic; 
    InstructionFamily family;
    InstructionSize size;
    EffectiveAddress src;
    EffectiveAddress dst;
    int32_t displacement;

} DecodedInstruction;


typedef int (*ExecFunc)(DecodedInstruction *di, M68kRegisters *registers, ReadByteFunc readByteFunc, ReadWordFunc readWordFunc,
    void *readWriteUserdata);

// Decode  instruction. Returns number of cycles or 0 if error
// The execution function is stored in execFunc
int decode(DecodedInstruction *di, M68kRegisters *registers, ReadByteFunc readByteFunc, ReadWordFunc readWordFunc,
    void *readWriteUserdata, ExecFunc *execFunc);