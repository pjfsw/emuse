#include "sourcedest.h"

static int execClr(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint32_t src;
    uint32_t dst;
    int cycleCount = writeDest(di, registers, rwFunc, readWriteUserdata, 0);
    if (cycleCount < 0) {
        return -1;        
    }
    return cycleCount;
}

static int execNot(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint32_t src;
    uint32_t dst;
    uint32_t value;
    int cycleCount = readSource(di, registers, &di->dst, rwFunc, readWriteUserdata, &value);
    if (cycleCount < 0) {
        return -1;
    }
    cycleCount+= writeDest(di, registers, rwFunc, readWriteUserdata, ~value);
    if (cycleCount < 0) {
        return -1;        
    }
    return cycleCount;
}

static int execNeg(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint32_t value;
    int cycleCount = readSource(di, registers, &di->dst, rwFunc, readWriteUserdata, &value);
    if (cycleCount < 0) {
        return -1;
    }

    uint32_t mask = di->size == 0 ? 0xff : di->size == 1 ? 0xffff : 0xffffffff;

    uint32_t signBit = (mask >> 1) + 1;
    uint32_t operand = value & mask;
    uint32_t result = (0u - operand) & mask;

    int writeCycles = writeDest(di, registers, rwFunc, readWriteUserdata, result);
    if (writeCycles < 0) {
        return -1;
    }

    cycleCount += writeCycles;

    // CCR:
    // X and C: set if operand != 0
    // V: set if negating minimum signed value
    // N: set if result is negative
    // Z: set if result is zero

    // Update CCR here using your existing flag helpers.

    return cycleCount;
}

static int decodeCommon(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    ReadWordFunc readWordFunc = rwFunc->rw;
    
    uint16_t mode = (opcode >> 3) & 7;
    uint16_t xn = opcode & 7;
    uint16_t size = (opcode >> 6) & 3;
    di->size = size;

    int eaCycles = getEffectiveAddress(registers, mode, xn, size, &di->dst, readWordFunc, readWriteUserdata);
    if (eaCycles < 0) {
        return -1;
    }
    return eaCycles;
}

int decodeClr(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    di->mnemonic = "CLR";
    di->execFunc = execClr;

    return decodeCommon(opcode, di, registers, rwFunc, readWriteUserdata);
}

int decodeNot(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    di->mnemonic = "NOT";
    di->execFunc = execNot;

    return decodeCommon(opcode, di, registers, rwFunc, readWriteUserdata);
}

int decodeNeg(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    di->mnemonic = "NEG";
    di->execFunc = execNeg;
    return decodeCommon(opcode, di, registers, rwFunc, readWriteUserdata);
}    
