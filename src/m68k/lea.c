#include "lea.h"

static int executeLea(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    registers->a[di->dst.xn] = di->src.address;
    return 0;
}

int decodeLea(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    ReadWordFunc readWordFunc = rwFunc->rw;

    di->execFunc = executeLea;
    di->mnemonic = "LEA";
    uint16_t srcMode = (opcode >> 3) & 7;
    uint16_t srcReg = opcode & 7;
    uint16_t dstReg = (opcode >> 9) & 7;
    int eaCycles = getEffectiveAddress(registers, srcMode, srcReg, IS_LONG, &di->src, readWordFunc, readWriteUserdata);
    if (eaCycles < 0) {
        return 0;
    }
    getEffectiveAddress(registers, AM_AREG, dstReg, IS_LONG, &di->dst, readWordFunc, readWriteUserdata);
    return eaCycles;
}