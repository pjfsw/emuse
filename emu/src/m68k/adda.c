#include "adda.h"
#include "sourcedest.h"

static int executeAddaSuba(
    DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata, bool isSub) {
    uint32_t src;

    int cycles = readSource(di, registers, &di->src, rwFunc, readWriteUserdata, &src);

    if (cycles < 0) {
        return -1;
    }

    if (di->size == IS_WORD) {
        src = (uint32_t)(int32_t)(int16_t)src;
    }

    if (isSub) {
        registers->a[di->dst.xn] -= src;
    } else {
        registers->a[di->dst.xn] += src;
    }

    return cycles;
}

static int executeAdda(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    return executeAddaSuba(di, registers, rwFunc, readWriteUserdata, false);
}

static int executeSuba(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    return executeAddaSuba(di, registers, rwFunc, readWriteUserdata, true);
}

static int decodeAddaSuba(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    ReadWordFunc readWordFunc = rwFunc->rw;

    uint16_t srcMode = (opcode >> 3) & 7;
    uint16_t srcReg = opcode & 7;
    uint16_t opMode = (opcode >> 6) & 7;
    uint16_t dstReg = (opcode >> 9) & 7;

    /*
     * ADDA opmodes:
     *
     *   011 = ADDA.W
     *   111 = ADDA.L
     */
    if (opMode == 3) {
        di->size = IS_WORD;
    } else if (opMode == 7) {
        di->size = IS_LONG;
    } else {
        return -1;
    }

    int eaCycles = getEffectiveAddress(registers, srcMode, srcReg, di->size, &di->src, readWordFunc, readWriteUserdata);

    if (eaCycles < 0) {
        return -1;
    }

    di->dst.mode = AM_AREG;
    di->dst.xn = dstReg;

    /*
     * Add the internal execution cycles here, depending on how your
     * emulator accounts for the common instruction fetch/decode cycles.
     */
    if (di->size == IS_WORD) {
        return eaCycles + 4;
    }

    return eaCycles + 2;
}

int decodeAdda(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    di->mnemonic = "ADDA";
    di->execFunc = executeAdda;
    return decodeAddaSuba(opcode, di, registers, rwFunc, readWriteUserdata);
}

int decodeSuba(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    di->mnemonic = "SUBA";
    di->execFunc = executeSuba;
    return decodeAddaSuba(opcode, di, registers, rwFunc, readWriteUserdata);
}