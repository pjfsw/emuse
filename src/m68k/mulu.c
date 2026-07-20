#include "mulu.h"
#include "sourcedest.h"

static int executeMulu(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint32_t src;

    int cycles = readSource(di, registers, &di->src, rwFunc, readWriteUserdata, &src);

    if (cycles < 0) {
        return -1;
    }

    uint32_t lhs = registers->d[di->dst.xn] & 0xffff;
    uint32_t rhs = src & 0xffff;
    uint32_t result = lhs * rhs;

    registers->d[di->dst.xn] = result;

    setFlag(registers, SR_FLAGS_N, (result & 0x80000000) != 0);
    setFlag(registers, SR_FLAGS_Z, result == 0);
    setFlag(registers, SR_FLAGS_V, false);
    setFlag(registers, SR_FLAGS_C, false);
    /* X unchanged */

    return cycles + 70; /* approximate for now */
}

int decodeMulu(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint16_t dstReg = (opcode >> 9) & 7;
    uint16_t srcMode = (opcode >> 3) & 7;
    uint16_t srcReg = opcode & 7;

    di->mnemonic = "MULU";
    di->execFunc = executeMulu;
    di->size = IS_WORD;

    di->dst.mode = AM_DREG;
    di->dst.xn = dstReg;

    return getEffectiveAddress(registers, srcMode, srcReg, IS_WORD, &di->src, rwFunc->rw, readWriteUserdata);
}