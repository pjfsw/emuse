#include "move.h"
#include "sourcedest.h"

static int executeMove(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint32_t value;
    int cycleCount = readSource(di, registers, &di->src, rwFunc, readWriteUserdata, &value);
    if (cycleCount < 0) {
        return -1;
    }
    if (!isTargetAddressRegister(di)) {
        setFlag(registers, SR_FLAGS_C, 0);
        setFlag(registers, SR_FLAGS_V, 0);
    }
    cycleCount += writeDest(di, registers, rwFunc, readWriteUserdata, value);
    return cycleCount;
}

int decodeMove(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    ReadWordFunc readWordFunc = rwFunc->rw;
    di->execFunc = executeMove;
    di->mnemonic = "MOVE";
    //  move.x
    uint16_t size = (opcode >> 12);
    if (size == 1) {
        size = IS_BYTE;
    } else if (size == 3) {
        size = IS_WORD;
    } else if (size == 2) {
        size = IS_LONG;
    }
    di->size = size;
    uint16_t srcMode = (opcode >> 3) & 7;
    uint16_t srcReg = opcode & 7;
    uint16_t dstMode = (opcode >> 6) & 7;
    uint16_t dstReg = (opcode >> 9) & 7;
    int eaCycles = getEffectiveAddress(registers, srcMode, srcReg, size, &di->src, readWordFunc, readWriteUserdata);
    if (eaCycles < 0) {
        return 0;
    }
    int cycles = eaCycles;

    eaCycles = getEffectiveAddress(registers, dstMode, dstReg, size, &di->dst, readWordFunc, readWriteUserdata);
    if (eaCycles < 0) {
        return 0;
    }
    cycles += eaCycles;
    return cycles;
}

int decodeMoveq(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    ReadWordFunc readWordFunc = rwFunc->rw;
    di->execFunc = executeMove;
    di->mnemonic = "MOVEQ";
    uint16_t dstReg = (opcode >> 9) & 7;
    di->size = IS_LONG;
    di->src.mode = AM_EXT;
    di->src.xn = AM_EXT_IMMEDIATE;
    di->src.immediate = (int32_t)(int8_t)(opcode);
    int eaCycles = getEffectiveAddress(registers, AM_DREG, dstReg, IS_LONG, &di->dst, readWordFunc, readWriteUserdata);
    if (eaCycles < 0) {
        return 0;
    }
    return eaCycles;
}
