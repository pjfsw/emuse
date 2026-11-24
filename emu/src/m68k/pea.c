#include "pea.h"
#include "pushpop.h"

static int executePea(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint32_t ea = di->src.address;
    pushWord(registers, rwFunc, readWriteUserdata, ea & 0xffff);
    pushWord(registers, rwFunc, readWriteUserdata, (ea >> 16) & 0xffff);
    return 8;
}

int decodePea(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    ReadWordFunc readWordFunc = rwFunc->rw;

    di->execFunc = executePea;
    di->mnemonic = "PEA";
    uint16_t srcMode = (opcode >> 3) & 7;
    uint16_t srcReg = opcode & 7;
    uint16_t dstReg = (opcode >> 9) & 7;
    int eaCycles = getEffectiveAddress(registers, srcMode, srcReg, IS_LONG, &di->src, readWordFunc, readWriteUserdata);
    if (eaCycles < 0) {
        return 0;
    }
    return eaCycles;
}
