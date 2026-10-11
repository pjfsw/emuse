#include "btst.h"

#include <stdio.h>

#include "sourcedest.h"

static int executeBit(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint32_t value;
    int cycles = readSource(di, registers, &di->dst, rwFunc, readWriteUserdata, &value);
    if (cycles < 0) {
        return -1;
    }

    uint32_t bit;
    if (di->src.mode == AM_DREG) {
        bit = registers->d[di->src.xn];
    } else {
        bit = di->src.immediate; 
    }

    bit &= (di->dst.mode == AM_DREG) ? 31 : 7;

    uint32_t mask = 1u << bit;

    // Z reflects the ORIGINAL bit
    uint16_t z = (value & mask) == 0;

    uint32_t result;

    switch (di->opcode & 0x00c0) {
        case 0x0000:  // BTST
            setFlag(registers, SR_FLAGS_Z, z);        
            return cycles;

        case 0x0080:  // BCLR
            result = value & ~mask;
            break;

        case 0x00c0:  // BSET
            result = value | mask;
            break;

        default:
            return -1;
    }

    int writeCycles = writeDest(di, registers, rwFunc, readWriteUserdata, result);
    if (writeCycles < 0) {
        return -1;
    }
    setFlag(registers, SR_FLAGS_Z, z);    

    return cycles + writeCycles;
}

int decodeBtstImmediate(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    ReadWordFunc readWordFunc = rwFunc->rw;

    uint16_t dstMode = (opcode >> 6) & 7;
    uint16_t dstReg = (opcode >> 9) & 7;
    uint16_t srcMode = (opcode >> 3) & 7;
    uint16_t srcReg = opcode & 7;
    di->execFunc = executeBit;
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
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    ReadWordFunc readWordFunc = rwFunc->rw;

    uint16_t bitReg = (opcode >> 9) & 7;
    uint16_t dstMode = (opcode >> 3) & 7;
    uint16_t dstReg = opcode & 7;

    di->execFunc = executeBit;
    di->mnemonic = "BTST";
    di->size = (dstMode == AM_DREG) ? IS_LONG : IS_BYTE;

    di->src.mode = AM_DREG;
    di->src.xn = bitReg;

    return getEffectiveAddress(registers, dstMode, dstReg, di->size, &di->dst, readWordFunc, readWriteUserdata);
}


int decodeBsetImmediate(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    int cycles = decodeBtstImmediate(opcode, di, registers, rwFunc, readWriteUserdata);
    di->mnemonic = "BSET";
    di->execFunc = executeBit;
    return cycles;
}

int decodeBset(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    int cycles = decodeBtst(opcode, di, registers, rwFunc, readWriteUserdata);
    di->mnemonic = "BSET";
    di->execFunc = executeBit;
    return cycles;
}


int decodeBclrImmediate(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    int cycles = decodeBtstImmediate(opcode, di, registers, rwFunc, readWriteUserdata);
    di->mnemonic = "BCLR";
    di->execFunc = executeBit;
    return cycles;
}

int decodeBclr(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    int cycles = decodeBtst(opcode, di, registers, rwFunc, readWriteUserdata);
    di->mnemonic = "BCLR";
    di->execFunc = executeBit;
    return cycles;
}
