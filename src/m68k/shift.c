#include "shift.h"
#include "sourcedest.h"
#include <stdio.h>

typedef void (*ShiftFunc)(uint32_t *value, M68kRegisters *registers, uint32_t mask, int bits, int count);

static void rotateLeft(uint32_t *value, M68kRegisters *registers, uint32_t mask, int bits, int count) {
    uint32_t v = *value;
    v = ((v << count) | (v >> (bits - count))) & mask;
    setFlag(registers, SR_FLAGS_C, (v & 1) != 0);
    *value = v;
}

static void rotateRight(uint32_t *value, M68kRegisters *registers, uint32_t mask, int bits, int count) {
    uint32_t v = *value;
    v = ((v >> count) | (v << (bits - count))) & mask;
    setFlag(registers, SR_FLAGS_C,  (v & (1 << (bits-1))) != 0);
    *value = v;
}

static int rotate(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata, ShiftFunc shiftFunc) {
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
        shiftFunc(&value, registers, mask, bits, count);
        setFlag(registers, SR_FLAGS_V, 0);
    }
    writeDest(di, registers, rwFunc, readWriteUserdata, value);

    cycleCount += 2 + di->src.immediate * 2;
    return cycleCount;
}

static int executeRol(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    return rotate(di, registers, rwFunc, readWriteUserdata, rotateLeft);
}

static int executeRor(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    return rotate(di, registers, rwFunc, readWriteUserdata, rotateRight);
}

static void setShiftModeSizeAndValue(uint16_t opcode, M68kRegisters *registers, EffectiveAddress *ea, InstructionSize *size) {
    uint16_t source = (opcode >> 5) & 1;
    uint16_t amount = (opcode >> 9) & 7;
    if (source == 0) {
        ea->immediate = (amount == 0) ? 8 : amount;
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

static void shiftLeft(uint32_t *value, M68kRegisters *registers, uint32_t mask, int bits, int count) {
    uint32_t v = *value;
    bool carry;

    if (count < bits) {
        carry = ((v >> (bits - count)) & 1) != 0;
        v = (v << count) & mask;
    } else if (count == bits) {
        carry = (v & 1) != 0;
        v = 0;
    } else {
        carry = false;
        v = 0;
    }

    setFlag(registers, SR_FLAGS_C, carry);
    setFlag(registers, SR_FLAGS_X, carry);
    *value = v;
}

static void shiftRight(uint32_t *value, M68kRegisters *registers, uint32_t mask, int bits, int count) {
    uint32_t v = *value;
    bool carry;

    if (count < bits) {
        carry = ((v >> (count - 1)) & 1) != 0;
        v >>= count;
    } else if (count == bits) {
        carry = ((v >> (bits - 1)) & 1) != 0;
        v = 0;
    } else {
        carry = false;
        v = 0;
    }

    setFlag(registers, SR_FLAGS_C, carry);
    setFlag(registers, SR_FLAGS_X, carry);
    *value = v & mask;
}

static int executeShift(
    DecodedInstruction *di,
    M68kRegisters *registers,
    RwFunc *rwFunc,
    void *readWriteUserdata,
    ShiftFunc shiftFunc)
{
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
    int count = di->src.immediate;

    setFlag(registers, SR_FLAGS_V, false);

    if (count == 0) {
        /* LSL/LSR with register count zero clear C but leave X unchanged. */
        setFlag(registers, SR_FLAGS_C, false);
    } else {
        shiftFunc(&value, registers, mask, bits, count);
    }

    writeDest(di, registers, rwFunc, readWriteUserdata, value);

    return cycleCount + 2 + count * 2;
}


static int executeLsl(DecodedInstruction *di, M68kRegisters *registers,
                      RwFunc *rwFunc, void *userdata)
{
    return executeShift(di, registers, rwFunc, userdata, shiftLeft);
}

static int executeLsr(DecodedInstruction *di, M68kRegisters *registers,
                      RwFunc *rwFunc, void *userdata)
{
    return executeShift(di, registers, rwFunc, userdata, shiftRight);
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
    di->mnemonic = "ROR";
    di->execFunc = executeRor;
    setShiftModeSizeAndValue(opcode, registers, &di->src, &di->size);
    setShiftTargetRegister(opcode, &di->dst);
    return 0;
}

int decodeRol(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    di->mnemonic = "ROL";
    di->execFunc = executeRol;
    setShiftModeSizeAndValue(opcode, registers, &di->src, &di->size);
    setShiftTargetRegister(opcode, &di->dst);
    return 0;
}

int decodeLsl(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    di->mnemonic = "LSL";
    di->execFunc = executeLsl;
    setShiftModeSizeAndValue(opcode, registers, &di->src, &di->size);
    setShiftTargetRegister(opcode, &di->dst);    
/*
    printf(
        "Decode LSL opcode=%04x sizeBits=%u size=%d count=%u dst=D%d\n",
        opcode,
        (opcode >> 6) & 3,
        di->size,
        di->src.immediate,
        di->dst.xn
    );    */
    return 0;
}

int decodeLsr(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    di->mnemonic = "LSR";
    di->execFunc = executeLsr;
    setShiftModeSizeAndValue(opcode, registers, &di->src, &di->size);
    setShiftTargetRegister(opcode, &di->dst);    
    return 0;
}