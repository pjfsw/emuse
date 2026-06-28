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


int decodeClr(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    ReadWordFunc readWordFunc = rwFunc->rw;

    di->mnemonic = "CLR";
    di->execFunc = execClr;
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
