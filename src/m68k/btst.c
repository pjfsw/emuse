#include "btst.h"
#include "sourcedest.h"

#include <stdio.h>

static int executeBtst(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint32_t value;
    int cycleCount = readSource(di, registers, &di->dst, rwFunc, readWriteUserdata, &value);

    int bit;
    if (di->src.mode == AM_DREG) {
        bit = registers->d[di->src.xn];  // Adapt register field name
    } else {
        bit = di->src.immediate;
    }

    bit &= (di->dst.mode == AM_DREG) ? 31 : 7;
    int isSet = value & (1 << bit);
    
    setFlag(registers, SR_FLAGS_Z, isSet == 0);
    if (cycleCount < 0) {
        return -1;
    }

    return cycleCount;
}

int decodeBtstImmediate(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    ReadWordFunc readWordFunc = rwFunc->rw;

    uint16_t dstMode = (opcode >> 6) & 7;
    uint16_t dstReg = (opcode >> 9) & 7;
    uint16_t srcMode = (opcode >> 3) & 7;
    uint16_t srcReg = opcode & 7;
    di->execFunc = executeBtst;
    di->mnemonic = "BTST";
    uint16_t size = (srcMode == AM_DREG) ? IS_WORD : IS_BYTE;
    di->size = size;
    di->src.mode = AM_EXT;
    di->src.xn = AM_EXT_IMMEDIATE;
    int eaCycles =
        getEffectiveAddress(registers, di->src.mode, di->src.xn, size, &di->src, readWordFunc, readWriteUserdata);
    if (eaCycles < 0) {
        return 0;
    }
    int cycles = eaCycles;
    eaCycles = getEffectiveAddress(registers, srcMode, srcReg, size, &di->dst, readWordFunc, readWriteUserdata);
    if (eaCycles < 0) {
        return 0;
    }
    cycles += eaCycles;
    return cycles;
}

int decodeBtst(
    uint16_t opcode, DecodedInstruction *di,
    M68kRegisters *registers, RwFunc *rwFunc,
    void *readWriteUserdata)
{
    ReadWordFunc readWordFunc = rwFunc->rw;

    uint16_t bitReg = (opcode >> 9) & 7;
    uint16_t dstMode = (opcode >> 3) & 7;
    uint16_t dstReg = opcode & 7;

    di->execFunc = executeBtst;
    di->mnemonic = "BTST";
    di->size = (dstMode == AM_DREG) ? IS_LONG : IS_BYTE;

    di->src.mode = AM_DREG;
    di->src.xn = bitReg;

    return getEffectiveAddress(registers, dstMode, dstReg, di->size, &di->dst, readWordFunc, readWriteUserdata);
}

