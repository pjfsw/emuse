#include "sourcedest.h"

static int execTst(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint32_t src;
    int cycleCount = readSource(di, registers, &di->src, rwFunc, readWriteUserdata, &src);
    if (cycleCount < 0) {
        return -1;
    }
    InstructionSize size = di->size;
    if (size == IS_BYTE) {
        setFlag(registers, SR_FLAGS_Z, (src & 0xFF) == 0);
        setFlag(registers, SR_FLAGS_N, (src & 0x80) != 0);
    } else if (size == IS_WORD) {
        setFlag(registers, SR_FLAGS_Z, (src & 0xFFFF) == 0);
        setFlag(registers, SR_FLAGS_N, (src & 0x8000) != 0);
    } else {
        setFlag(registers, SR_FLAGS_Z, src == 0);
        setFlag(registers, SR_FLAGS_N, (src & 0x80000000) != 0);
    }
    setFlag(registers, SR_FLAGS_C, false);
    setFlag(registers, SR_FLAGS_V, false);
    
    return cycleCount;
}


int decodeTst(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    ReadWordFunc readWordFunc = rwFunc->rw;

    di->mnemonic = "TST";
    di->execFunc = execTst;
    uint16_t mode = (opcode >> 3) & 7;
    uint16_t xn = opcode & 7;
    uint16_t size = (opcode >> 6) & 3;
    di->size = size;

    int eaCycles = getEffectiveAddress(registers, mode, xn, size, &di->src, readWordFunc, readWriteUserdata);
    if (eaCycles < 0) {
        return -1;
    }
    return eaCycles;
}
