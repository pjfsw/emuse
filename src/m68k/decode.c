#include "decode.h"
#include "sourcedest.h"
#include "move.h"
#include "branch.h"
#include "rts.h"
#include "btst.h"
#include <stdio.h>

int getEffectiveAddress(M68kRegisters *registers, uint16_t mode, uint16_t reg, InstructionSize size,
    EffectiveAddress *ea, ReadWordFunc readWordFunc, void *readWriteUserdata) {

    ea->mode = mode;
    ea->xn = reg;

    if ((mode == AM_DREG) || (mode == AM_AREG)) { // d0, a0
        return 0;
    } else if ((mode == AM_ADDRESS) || (mode == AM_ADDRESS_POST_INC)) { // (a0) or (a0)+
        ea->address = registers->a[reg];
        return 0;        
    } else if (mode == AM_ADDR_DISP) { // disp(a0)
        ea->displacement = (int16_t)readWordFunc(readWriteUserdata, registers->pc);
        increasePc(registers);
        ea->address = (int32_t)ea->displacement + registers->a[reg];
        return 4;
    } else if (mode == AM_EXT) {
        if (reg == AM_EXT_IMMEDIATE) {
            // Immediate
            if (size == IS_LONG) {
                ea->immediate = ((uint32_t)readWordFunc(readWriteUserdata, registers->pc)) << 16;
                increasePc(registers);
                ea->immediate |= (uint32_t)readWordFunc(readWriteUserdata, registers->pc);
                increasePc(registers);
                return 8;
            } else {
                ea->immediate = readWordFunc(readWriteUserdata, registers->pc);
                if (size == IS_BYTE) {
                    ea->immediate = ea->immediate & 0xff;
                }
                increasePc(registers);
                return 4;
            }
        } else if (reg == AM_EXT_ABS_LONG) {
            ea->address = ((uint32_t)readWordFunc(readWriteUserdata, registers->pc)) << 16;
            increasePc(registers);
            ea->address |= (uint32_t)readWordFunc(readWriteUserdata, registers->pc);
            increasePc(registers);
            return 8;
        } else if (reg == AM_EXT_PC_DISP) {
            int16_t displacement = (int16_t)readWordFunc(readWriteUserdata, registers->pc);
            ea->address = registers->pc + (int32_t)displacement;
            increasePc(registers);
            return 4;
        }
    }
    return -1;
}

void setFlag(M68kRegisters *registers, uint16_t flag, uint16_t value) {
    registers->sr = (registers->sr & (0xffff-flag)) | (value * flag);
}

bool getFlag(M68kRegisters *registers, uint16_t flag) {
    return (registers->sr & flag) != 0;
}

void setNZ(M68kRegisters *registers, int32_t value) {
    setFlag(registers, SR_FLAGS_N, value < 0);
    setFlag(registers, SR_FLAGS_Z, value == 0);
}

static int executeLea(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    registers->a[di->dst.xn] = di->src.address;
    return 0;
}

static uint32_t aluSub(uint32_t a, uint32_t b, uint16_t size, M68kRegisters *regs) {
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

static char *mn_add = "ADD";
static char *mn_addx = "ADDX";
static char *mn_addq = "ADDQ";
static char *mn_cmp = "CMP";
static char *mn_cmpa = "CMPA";
static char *mn_subq = "SUBQ";
static char *mn_lea = "LEA";
static char *mn_unknown = "???";

typedef struct {
    uint16_t mask;
    uint16_t value;
    DecodeFunc decodeFunc;
} DecodeRule;

static const DecodeRule rules[] = {
    { 0xffff, 0x4e75, decodeRts },
    { 0xffc0, 0x0800, decodeBtstImmediate}, 
    { 0xf000, 0x1000, decodeMove },
    { 0xf000, 0x2000, decodeMove },
    { 0xf000, 0x3000, decodeMove },
    { 0xf000, 0x7000, decodeMoveq },
    { 0xf000, 0x6000, decodeBranch },
};

int decode(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    ReadWordFunc readWordFunc = rwFunc->rw;
    uint16_t cycles = 0;
    uint16_t opcode = readWordFunc(readWriteUserdata, registers->pc);
    di->opcode = opcode;
    increasePc(registers);
    cycles += 4;    

    di->mnemonic = mn_unknown;
    uint16_t family = opcode & 0xf000;

    for (int i = 0 ; i < sizeof(rules)/sizeof(DecodeRule); i++) {
        if ((opcode & rules[i].mask) == rules[i].value) {
            int decodeCycles = rules[i].decodeFunc(opcode, di, registers, rwFunc, readWriteUserdata);
            if (decodeCycles < 0) {
                return 0;
            } else {
                return cycles + decodeCycles;
            }
        }
    }

    if (family == 0xb000) {
        di->family = IF_MOVE;
        di->execFunc = executeCmp;
        uint16_t mode = (opcode >> 3) & 7;
        uint16_t srcReg = opcode & 7;
        uint16_t opMode = (opcode >> 6) & 7;
        uint16_t dstReg = (opcode >> 9) & 7;  
        if (opMode <= 2) {
            // CMP.x
            di->mnemonic = mn_cmp;
            uint16_t size = opMode & 3;
            di->size = size;
            int eaCycles = getEffectiveAddress(registers, mode, srcReg, size, &di->src, readWordFunc, readWriteUserdata);
            if (eaCycles < 0) {
                return 0;
            }
            di->dst.mode = AM_DREG;
            di->dst.xn = dstReg;
            di->aluFunc = aluSub;
            cycles += eaCycles;
            if (di->size == IS_LONG) {
                cycles += 2;
            }
        } else if ((opMode == 3) || (opMode == 7)) {
            // CMPA
            di->mnemonic = mn_cmpa;
            if (opMode == 3) {
                di->size = IS_WORD;
            } else {
                di->size = IS_LONG;
            }
            int eaCycles = getEffectiveAddress(registers, mode, srcReg, di->size, &di->src, readWordFunc, readWriteUserdata);
            if (eaCycles < 0) {
                return 0;
            }
            di->dst.mode = AM_AREG;
            di->dst.xn = dstReg;
            di->aluFunc = aluSub;
            cycles += 2;
            cycles += eaCycles;
        } else {
            // CMPM
            return 0;            
        }
    } else if (family == 0xd000) { // ADD
        di->family = IF_MOVE;
        di->mnemonic = mn_add;
        di->execFunc = executeAlu;
        di->aluFunc = aluAdd;
        uint16_t mode = (opcode >> 3) & 7;
        uint16_t srcReg = opcode & 7;
        uint16_t opMode = (opcode >> 6) & 7;
        uint16_t dstReg = (opcode >> 9) & 7;  
        if (((opMode == 4) || (opMode == 5) || (opMode == 6)) && ((mode == 0) || (mode == 1))) { // ADDX
            uint16_t size = opMode & 3;
            di->size = size;
            di->mnemonic = mn_addx;            
            di->aluFunc = aluAddx;
            int eaCycles = 0;
            if (mode == 0) { // Dn,Dn
                di->src.mode = AM_DREG;
                di->src.xn = srcReg;
                di->dst.mode = AM_DREG;
                di->dst.xn = dstReg;
            } else { // ( -(An),-(An)
                return 0; // TODO
            }            
            cycles += eaCycles;
        } else { // Normal ADD            
            // Byte = 00, Word = 01, Long = 10
            uint16_t size = opMode & 3;
            di->size = size;
            EffectiveAddress ea;
            int eaCycles = getEffectiveAddress(registers, mode, srcReg, size, &ea, readWordFunc, readWriteUserdata);
            EffectiveAddress dr = {
                .mode = AM_DREG,
                .xn = dstReg
            };            
            uint16_t direction = opMode >> 2;
            if (!direction) { // Dn + EA => Dn
                di->src = ea;
                di->dst = dr;
            } else {         // EA + Dn => EA
                di->src = dr;
                di->dst = ea;                
            }            
            if (di->size == IS_LONG) {
                cycles += 2;
            }
            cycles += eaCycles;
        }
    } else if (family == 0x5000) { 
        uint16_t size = (opcode >> 6) & 3;
        uint16_t dstMode = (opcode >> 3) & 7;
        uint16_t dstReg = opcode & 7;
        if (size == 3) { // Scc, DBcc
            return 0;
        } else { // ADDQ/SUBQ
            uint16_t isSub = (opcode >> 8) & 1;
            uint16_t data = ((opcode >> 9) & 7);   
            if (data == 0) {
                data = 8;
            }
            int eaCycles = getEffectiveAddress(registers, dstMode, dstReg, size, &di->dst, readWordFunc, readWriteUserdata);
            di->src.immediate = (uint32_t)data;
            di->src.mode = AM_EXT;
            di->src.xn = AM_EXT_IMMEDIATE;
            di->size = size;
            di->family = IF_MOVE;
            di->execFunc = executeAlu;
            if (isSub) {
                di->mnemonic = mn_subq;
                di->aluFunc = aluSub;
            } else {
                di->mnemonic = mn_addq;
                di->aluFunc = aluAdd;
            }
            cycles += eaCycles;
        }
    } else if ((opcode & 0xf1c0) == 0x41c0) { // LEA
        di->execFunc = executeLea;
        di->family = IF_LEA;
        di->mnemonic = mn_lea;
        uint16_t srcMode = (opcode >> 3) & 7;
        uint16_t srcReg = opcode & 7;
        uint16_t dstReg = (opcode >> 9) & 7;        
        int eaCycles = getEffectiveAddress(registers, srcMode, srcReg, IS_LONG, &di->src, readWordFunc, readWriteUserdata);
        if (eaCycles < 0) {
            return 0;
        }
        getEffectiveAddress(registers,AM_AREG,dstReg, IS_LONG, &di->dst, readWordFunc, readWriteUserdata);
        cycles += eaCycles;
        return cycles;
    }
    return cycles;
}
