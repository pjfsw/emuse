#include "alu.h"
#include "sourcedest.h"

static void setAddFlags(uint64_t result, uint16_t size, M68kRegisters *regs) {
    uint64_t realResult;
    if (size == IS_BYTE) {
        realResult = (uint8_t)result;
    } else if (size == IS_WORD) {
        realResult = (uint16_t)result;
    } else {
        realResult = (uint32_t)result;
    }
    bool carry = (uint64_t)result != (uint64_t)realResult;
    setFlag(regs, SR_FLAGS_C, carry);
    setFlag(regs, SR_FLAGS_X, carry);
    //printf("%lx %lx %d %x\n", result, realResult, carry, regs->sr);
}


static uint32_t aluSub(uint32_t a, uint32_t b, uint16_t size, M68kRegisters *regs) {
    uint32_t mask;
    if (size == IS_BYTE) {
        mask = 0xff;
    } else if (size == IS_WORD) {
        mask = 0xffff;
    } else {
        mask = 0xffffffff;
    }
    a &= mask;
    b &= mask;
    uint64_t result = (uint64_t)((uint64_t)b - (uint64_t)a);    
    uint64_t realResult;
    uint32_t signMask;
    if (size == IS_BYTE) {

        realResult = (uint8_t)result;
        signMask = 0x80;
    } else if (size == IS_WORD) {
        realResult = (uint16_t)result;
        signMask = 0x8000;
    } else {
        realResult = (uint32_t)result;
        signMask = 0x80000000;
    }
    bool carry = (uint64_t)result != (uint64_t)realResult;
    setFlag(regs, SR_FLAGS_C, carry);
    setFlag(regs, SR_FLAGS_X, carry); // SHOULD NOT BE SET BY CMP
    setFlag(regs, SR_FLAGS_Z, realResult == 0);
    setFlag(regs, SR_FLAGS_N, (realResult & signMask) != 0);
    bool signA = (a & signMask) != 0;
    bool signB = (b & signMask) != 0;
    bool signR = (realResult & signMask) != 0;    
    setFlag(regs, SR_FLAGS_V, (signB != signA) && (signR != signB));
    //printf("%lx %lx %d %x\n", result, realResult, carry, regs->sr);
    return (uint32_t)result;
}

static int executeCmp(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint32_t src;
    uint32_t dst;
    int cycleCount = readSource(di, registers, &di->src, rwFunc, readWriteUserdata, &src);
    if (cycleCount < 0) {
        return -1;
    }
    int destCycleCount = readSource(di, registers, &di->dst, rwFunc, readWriteUserdata, &dst);
    if (destCycleCount < 0) {
        return -1;
    }
    cycleCount += destCycleCount;
    bool saveX = getFlag(registers, SR_FLAGS_X); // Ugly hack since X is unaffected by CMP
    dst = di->aluFunc(src,dst, di->size, registers);
    setFlag(registers, SR_FLAGS_X, saveX);
    return cycleCount;
}

static uint32_t aluAnd(uint32_t a, uint32_t b, uint16_t size, M68kRegisters *regs) {
    uint64_t result = (uint64_t)((uint64_t)a & (uint64_t)b);    
    setFlag(regs, SR_FLAGS_V, false);
    setFlag(regs, SR_FLAGS_C, false);
    return (uint32_t)result;
}

static uint32_t aluOr(uint32_t a, uint32_t b, uint16_t size, M68kRegisters *regs) {
    uint64_t result = (uint64_t)((uint64_t)a | (uint64_t)b);    
    setFlag(regs, SR_FLAGS_V, false);
    setFlag(regs, SR_FLAGS_C, false);
    return (uint32_t)result;
}

static uint32_t aluAdd(uint32_t a, uint32_t b, uint16_t size, M68kRegisters *regs) {
    uint64_t result = (uint64_t)((uint64_t)a + (uint64_t)b);    
    setAddFlags(result, size, regs);
    return (uint32_t)result;
}

static uint32_t aluAddx(uint32_t a, uint32_t b, uint16_t size, M68kRegisters *regs) {
    uint64_t x = getFlag(regs, SR_FLAGS_X) ? 1 : 0;
    uint64_t result = (uint64_t)((uint64_t)a + (uint64_t)b + x);    
    setAddFlags(result, size, regs);
    return (uint32_t)result;
}

static int executeAlu(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint32_t src;
    uint32_t dst;
    int cycleCount = readSource(di, registers, &di->src, rwFunc, readWriteUserdata, &src);
    if (cycleCount < 0) {
        return -1;
    }
    int destCycleCount = readSource(di, registers, &di->dst, rwFunc, readWriteUserdata, &dst);
    if (destCycleCount < 0) {
        return -1;
    }
    cycleCount += destCycleCount;
    dst = di->aluFunc(src,dst, di->size, registers);
    int writeCycleCount = writeDest(di, registers, rwFunc, readWriteUserdata, dst);
    if (writeCycleCount < 0) {
        return -1;        
    }
    cycleCount += writeCycleCount;
    return cycleCount;
}


int decodeAddx(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    di->mnemonic = "ADDX";
    di->execFunc = executeAlu;
    di->aluFunc = aluAdd;
    uint16_t mode = (opcode >> 3) & 7;
    uint16_t srcReg = opcode & 7;
    uint16_t opMode = (opcode >> 6) & 7;
    uint16_t dstReg = (opcode >> 9) & 7;
    uint16_t size = opMode & 3;
    di->size = size;
    di->mnemonic = "ADDX";
    di->aluFunc = aluAddx;
    int eaCycles = 0;
    if (mode == 0) {  // Dn,Dn
        di->src.mode = AM_DREG;
        di->src.xn = srcReg;
        di->dst.mode = AM_DREG;
        di->dst.xn = dstReg;
    } else {       // ( -(An),-(An)
        return 0;  // TODO
    }
    return eaCycles;
}

int decodeAddqSubq(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    ReadWordFunc readWordFunc = rwFunc->rw;

    uint16_t size = (opcode >> 6) & 3;
    uint16_t dstMode = (opcode >> 3) & 7;
    uint16_t dstReg = opcode & 7;
    uint16_t isSub = (opcode >> 8) & 1;
    uint16_t data = ((opcode >> 9) & 7);
    if (data == 0) {
        data = 8;
    }
    int eaCycles = getEffectiveAddress(registers, dstMode, dstReg, size, &di->dst, readWordFunc, readWriteUserdata);
    if (eaCycles < 0) {
        return -1;
    }
    di->src.immediate = (uint32_t)data;
    di->src.mode = AM_EXT;
    di->src.xn = AM_EXT_IMMEDIATE;
    di->size = size;
    di->execFunc = executeAlu;
    if (isSub) {
        di->mnemonic = "SUBQ";
        di->aluFunc = aluSub;
    } else {
        di->mnemonic = "ADDQ";
        di->aluFunc = aluAdd;
    }
    return eaCycles;
}


int decodeCmp(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    ReadWordFunc readWordFunc = rwFunc->rw;

    di->execFunc = executeCmp;
    uint16_t mode = (opcode >> 3) & 7;
    uint16_t srcReg = opcode & 7;
    uint16_t opMode = (opcode >> 6) & 7;
    uint16_t dstReg = (opcode >> 9) & 7;
    // CMP.x
    di->mnemonic = "CMP";
    uint16_t size = opMode & 3;
    di->size = size;
    int eaCycles = getEffectiveAddress(registers, mode, srcReg, size, &di->src, readWordFunc, readWriteUserdata);
    if (eaCycles < 0) {
        return -1;
    }
    di->dst.mode = AM_DREG;
    di->dst.xn = dstReg;
    di->aluFunc = aluSub;
    int cycles = eaCycles;
    if (di->size == IS_LONG) {
        cycles += 2;
    }
    return cycles;
}

int decodeCmpa(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    ReadWordFunc readWordFunc = rwFunc->rw;

    di->execFunc = executeCmp;
    uint16_t mode = (opcode >> 3) & 7;
    uint16_t srcReg = opcode & 7;
    uint16_t opMode = (opcode >> 6) & 7;
    uint16_t dstReg = (opcode >> 9) & 7;

    di->mnemonic = "CMPA";
    if (opMode == 3) {
        di->size = IS_WORD;
    } else {
        di->size = IS_LONG;
    }
    int eaCycles = getEffectiveAddress(registers, mode, srcReg, di->size, &di->src, readWordFunc, readWriteUserdata);
    if (eaCycles < 0) {
        return -1;
    }
    di->dst.mode = AM_AREG;
    di->dst.xn = dstReg;
    di->aluFunc = aluSub;
    int cycles = 2;
    cycles += eaCycles;
    return cycles;
}

int decodeCmpm(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    return -1;
}

static int decodeAluImmediate(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    ReadWordFunc readWordFunc = rwFunc->rw;

    di->execFunc = executeAlu;
    uint16_t mode = (opcode >> 3) & 7;
    uint16_t xn = opcode & 7;
    uint16_t size = (opcode >> 6) & 3;

    di->size = size;
    int eaCycles = getEffectiveAddress(registers, mode, xn, size, &di->dst, readWordFunc, readWriteUserdata);
    if (eaCycles < 0) {
        return -1;
    }
    int eaCycles2 = getEffectiveAddress(registers, AM_EXT, AM_EXT_IMMEDIATE, size, &di->src, readWordFunc, readWriteUserdata);
    if (eaCycles2 < 0) {
        return -1;
    }

    return eaCycles + eaCycles2;
}

static int decodeAlu(
        uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    ReadWordFunc readWordFunc = rwFunc->rw;
       
    int cycles = 0;
    di->execFunc = executeAlu;
    uint16_t mode = (opcode >> 3) & 7;
    uint16_t srcReg = opcode & 7;
    uint16_t opMode = (opcode >> 6) & 7;
    uint16_t dstReg = (opcode >> 9) & 7;

    // Byte = 00, Word = 01, Long = 10
    uint16_t size = opMode & 3;
    di->size = size;
    EffectiveAddress ea;
    int eaCycles = getEffectiveAddress(registers, mode, srcReg, size, &ea, readWordFunc, readWriteUserdata);
    if (eaCycles < 0) {
        return -1;
    }
    EffectiveAddress dr = {.mode = AM_DREG, .xn = dstReg};
    uint16_t direction = opMode >> 2;
    if (!direction) {  // Dn + EA => Dn
        di->src = ea;
        di->dst = dr;
    } else {  // EA + Dn => EA
        di->src = dr;
        di->dst = ea;
    }
    if (di->size == IS_LONG) {
        cycles += 2;
    }
    cycles += eaCycles;
    return cycles;
}

int decodeAdd(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    di->mnemonic = "ADD";
    di->aluFunc = aluAdd;

    return decodeAlu(opcode, di, registers, rwFunc, readWriteUserdata);
}

int decodeSub(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    di->mnemonic = "SUB";
    di->aluFunc = aluSub;

    return decodeAlu(opcode, di, registers, rwFunc, readWriteUserdata);
}

int decodeAnd(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    di->mnemonic = "AND";
    di->aluFunc = aluAnd;

    return decodeAlu(opcode, di, registers, rwFunc, readWriteUserdata);
}

int decodeAndi(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    di->mnemonic = "ANDI";
    di->aluFunc = aluAnd;

    return decodeAluImmediate(opcode, di, registers, rwFunc, readWriteUserdata);
}

int decodeOr(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    di->mnemonic = "OR";
    di->aluFunc = aluOr;

    return decodeAlu(opcode, di, registers, rwFunc, readWriteUserdata);
}

int decodeOri(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    di->mnemonic = "ORI";
    di->aluFunc = aluOr;

    return decodeAluImmediate(opcode, di, registers, rwFunc, readWriteUserdata);
}
