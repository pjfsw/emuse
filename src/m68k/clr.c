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
