#include "adda.h"
#include "sourcedest.h"

static int executeAdda(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint32_t src;

    int cycles = readSource(di, registers, &di->src, rwFunc, readWriteUserdata, &src);

    if (cycles < 0) {
        return -1;
    }

    /*
     * ADDA.W sign-extends the 16-bit source.
     * ADDA.L uses the full 32-bit source.
     */
    if (di->size == IS_WORD) {
        src = (uint32_t)(int32_t)(int16_t)src;
    }

    registers->a[di->dst.xn] += src;

    return cycles;
}

int decodeAdda(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    ReadWordFunc readWordFunc = rwFunc->rw;

    uint16_t srcMode = (opcode >> 3) & 7;
    uint16_t srcReg = opcode & 7;
    uint16_t opMode = (opcode >> 6) & 7;
    uint16_t dstReg = (opcode >> 9) & 7;

    di->mnemonic = "ADDA";
    di->execFunc = executeAdda;

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