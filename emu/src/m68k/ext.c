#include "ext.h"

static int executeExtl(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint32_t *d = &registers->d[di->dst.xn];   

    *d = (uint32_t)(int32_t)(int16_t)(*d & 0xffff);

    setFlag(registers, SR_FLAGS_Z, *d == 0);
    setFlag(registers, SR_FLAGS_N, (*d & 0x80000000) != 0);
    setFlag(registers, SR_FLAGS_V, false);
    setFlag(registers, SR_FLAGS_C, false);

    return 0;
}

static int executeExtw(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint32_t *d = &registers->d[di->dst.xn];

    *d = (*d & 0xffff0000) | (uint16_t)(int16_t)(int8_t)(*d & 0xff);

    setFlag(registers, SR_FLAGS_Z, *d == 0);
    setFlag(registers, SR_FLAGS_N, (*d & 0x8000) != 0);
    setFlag(registers, SR_FLAGS_V, false);
    setFlag(registers, SR_FLAGS_C, false);

    return 0;
}

static int extCommon(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers) {
    di->mnemonic = "EXT";
    di->dst.mode = AM_DREG;
    di->dst.xn = opcode & 7;
    return 0;
}

int decodeExtw(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *userdata) {
    di->execFunc = executeExtw;
    di->size = IS_WORD;
    return extCommon(opcode, di, registers);
}

int decodeExtl(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *userdata) {
    di->execFunc = executeExtl;
    di->size = IS_LONG;
    return extCommon(opcode, di, registers);
}