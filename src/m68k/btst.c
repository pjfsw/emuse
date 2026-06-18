#include "btst.h"
#include "sourcedest.h"

static int executeBtst(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint32_t value;
    int cycleCount = readSource(di, registers, &di->dst, rwFunc, readWriteUserdata, &value);
    int isSet = value & (1 << di->src.immediate);
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
    di->family = IF_MOVE;
    di->mnemonic = "BTST";
    uint16_t size = (srcMode == AM_DREG) ? IS_LONG : IS_BYTE;
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
