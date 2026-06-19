#include "shift.h"
#include "sourcedest.h"

static int executeRol(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint32_t src;
    int cycleCount = readSource(di, registers, &di->dst, rwFunc, readWriteUserdata, &src);
    if (cycleCount < 0) {
        return -1;
    }

    uint32_t mask;
    int bits;
    if (di->size == IS_BYTE) {
        mask = 0xff;
        bits = 8;
    } else if (di->size == IS_WORD) {
        mask = 0xffff;
        bits = 16;
    } else {
        mask = 0xffffffff;
        bits = 32;
    }
    uint32_t value = src & mask;
    uint32_t count = di->src.immediate % bits;      
    if (count > 0) {
        value = ((value << count) | (value >> (bits - count))) & mask;
    }
    writeDest(di, registers, rwFunc, readWriteUserdata, value);

    cycleCount += 2 + di->src.immediate * 2;
    return cycleCount;
}


static void setShiftModeSizeAndValue(uint16_t opcode, M68kRegisters *registers, EffectiveAddress *ea, InstructionSize *size) {
    uint16_t source = (opcode >> 5) & 1;
    uint16_t amount = (opcode >> 9) & 7;
    if (source == 0) {
        ea->immediate = amount;
        ea->mode = AM_EXT;
        ea->xn = AM_EXT_IMMEDIATE;
    } else {
        ea->immediate = registers->d[amount] & 0x3f;        
        ea->mode = AM_DREG;
    }
    uint16_t sizeBits = (opcode >> 6) & 3;    
    if (sizeBits == 0) {
        *size = IS_BYTE;
    } else if (sizeBits == 1) {
        *size = IS_WORD;
    } else {
        *size = IS_LONG;
    }
}

static void setShiftTargetRegister(uint16_t opcode, EffectiveAddress *ea) {
    int dstReg = opcode & 7;
    ea->mode = AM_DREG;
    ea->xn = dstReg;
}

int decodeRoxrEa(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    return -1;
}


int decodeRoxlEa(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    return -1;
}

int decodeRoxr(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    return -1;
}

int decodeRoxl(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    return -1;
}


int decodeRor(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    return -1;
}

int decodeRol(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    di->mnemonic = "ROL";
    di->execFunc = executeRol;
    setShiftModeSizeAndValue(opcode, registers, &di->src, &di->size);
    setShiftTargetRegister(opcode, &di->dst);
    return 0;
}

