#include "swap.h"

static int executeSwap(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint32_t value = registers->d[di->dst.xn];

    value = (value << 16) | (value >> 16);
    registers->d[di->dst.xn] = value;

    setFlag(registers, SR_FLAGS_Z, value == 0);
    setFlag(registers, SR_FLAGS_N, (value & 0x80000000) != 0);
    setFlag(registers, SR_FLAGS_V, false);
    setFlag(registers, SR_FLAGS_C, false);

    return 4;
}

int decodeSwap(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *userdata) {
    di->execFunc = executeSwap;
    di->mnemonic = "SWAP";
    di->size = IS_LONG;

    di->dst.mode = AM_DREG;
    di->dst.xn = opcode & 7;

    return 4;
}