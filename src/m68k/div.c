#include "div.h"

#include "sourcedest.h"

static int executeDivu(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint32_t divisor;
    int cycleCount = readSource(di, registers, &di->src, rwFunc, readWriteUserdata, &divisor);
    if (cycleCount < 0) {
        return -1;
    }

    divisor = divisor & 0xffff;
    uint32_t dividend = registers->d[di->dst.xn];

    if (divisor == 0) {
        // TODO raise exception
        return -1;
    } 
    uint32_t quotient = dividend / divisor;
    uint32_t remainder = dividend % divisor;

    setFlag(registers, SR_FLAGS_C, false);
    setFlag(registers, SR_FLAGS_N, false);
    setFlag(registers, SR_FLAGS_Z, false);
    setFlag(registers, SR_FLAGS_V, false);

    if (quotient > 0xffff) {
        // Overflow
        setFlag(registers, SR_FLAGS_V, true);
        return cycleCount;
    }

    registers->d[di->dst.xn] = (remainder << 16) | (quotient & 0xffff);

    if (quotient == 0) {
        setFlag(registers, SR_FLAGS_Z, true);
    }

    if (quotient & 0x8000) {
        setFlag(registers, SR_FLAGS_N, true);
    }

    cycleCount += 140;
    return cycleCount;
}

int decodeDivu(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    ReadWordFunc readWordFunc = rwFunc->rw;

    uint16_t dstReg = (opcode >> 9) & 7;
    uint16_t srcMode = (opcode >> 3) & 7;
    uint16_t srcReg = opcode & 7;

    di->execFunc = executeDivu;
    di->mnemonic = "DIVU";
    di->size = IS_WORD;

    di->dst.mode = AM_DREG;
    di->dst.xn = dstReg;

    int eaCycles = getEffectiveAddress(registers, srcMode, srcReg, IS_WORD, &di->src, readWordFunc, readWriteUserdata);
    return eaCycles;
}


int decodeDivs(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    return -1;
}

