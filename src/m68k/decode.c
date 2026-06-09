#include "decode.h"
#include <stdio.h>

static inline uint32_t align24(uint32_t address) {
    return address & 0xffffff;
}

static inline void increasePc(M68kRegisters *registers) {
    registers->pc = align24(registers->pc + 2);
}

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

static void setFlag(M68kRegisters *registers, uint16_t flag, uint16_t value) {
    registers->sr = (registers->sr & (0xffff-flag)) | (value * flag);
}

static bool getFlag(M68kRegisters *registers, uint16_t flag) {
    return (registers->sr & flag) != 0;
}

static void setNZ(M68kRegisters *registers, int32_t value) {
    setFlag(registers, SR_FLAGS_N, value < 0);
    setFlag(registers, SR_FLAGS_Z, value == 0);
}

static int executeLea(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    registers->a[di->dst.xn] = di->src.address;
    return 0;
}

static void preDecrement(DecodedInstruction *di, M68kRegisters *registers, EffectiveAddress *ea) {
    if (ea->mode != AM_ADDRESS_PRE_DEC) {
        return;
    }
}

static void postIncrement(DecodedInstruction *di, M68kRegisters *registers, EffectiveAddress *ea) {
    if (ea->mode != AM_ADDRESS_POST_INC) {
        return;
    }
    int size = 0;
    if (di->size == IS_LONG) {
        size = 4;
    } else if (di->size == IS_WORD) {
        size = 2;
    } else {
        if (ea->xn == 7) {
            // Stack special case
            size = 2;
        } else {
            size = 1;
        }
    }
    registers->a[ea->xn] = align24(registers->a[ea->xn] + size);
}

static int readSource(
    DecodedInstruction *di, M68kRegisters *registers, EffectiveAddress *ea, RwFunc *rwFunc, void *readWriteUserdata, uint32_t *value) {
    int cycleCount = -1;
    if (ea->mode == AM_EXT) {
        if (ea->xn == AM_EXT_IMMEDIATE) {
            *value = ea->immediate;
            cycleCount = 0;
        } else if (ea->xn == AM_EXT_ABS_LONG) {
            if (di->size == IS_LONG) {
                *value = (uint32_t)rwFunc->rw(readWriteUserdata, ea->address) << 16;
                *value |= (uint32_t)rwFunc->rw(readWriteUserdata, ea->address + 2);
                cycleCount = 8;
            } else if (di->size == IS_WORD) {
                *value = rwFunc->rw(readWriteUserdata, ea->address);
                cycleCount = 4;
            } else {
                *value = rwFunc->rb(readWriteUserdata, ea->address);
                cycleCount = 4;
            }
        }
    } else if (ea->mode == AM_DREG) {
        *value = registers->d[ea->xn];
        cycleCount = 0;
    } else if (ea->mode == AM_AREG) {
        *value = registers->a[ea->xn];
        cycleCount = 0;
    } else if ((ea->mode == AM_ADDRESS) || (ea->mode == AM_ADDR_DISP) || (ea->mode == AM_ADDRESS_POST_INC)) {
        preDecrement(di, registers, ea);
//        printf("Read $%06x\n", di->src.address);
        cycleCount = 4;
        if (di->size == IS_BYTE) {
            *value = (uint32_t)rwFunc->rb(readWriteUserdata, ea->address);
        } else if (di->size == IS_WORD) {
            *value = (uint32_t)rwFunc->rw(readWriteUserdata, ea->address);
        } else if (di->size == IS_LONG) {
            *value = (uint32_t)rwFunc->rw(readWriteUserdata, ea->address) << 16;
            *value |= (uint32_t)rwFunc->rw(readWriteUserdata, ea->address + 2);
            cycleCount += 4;
        }
        postIncrement(di, registers, ea);

    }
    return cycleCount;
}

static inline bool isTargetAddressRegister(DecodedInstruction *di) {
    return di->dst.mode == AM_AREG;
}

static int writeDest(
    DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata, uint32_t value) {
    uint32_t cycleCount = -1;

    if ((di->dst.mode == AM_DREG)) {
        cycleCount = 0;
        if (di->size == IS_LONG) {
            setNZ(registers, (int32_t)value);
            registers->d[di->dst.xn] = value;
        } else if (di->size == IS_WORD) {
            setNZ(registers, (int32_t)(int16_t)(value & 0xffff));
            registers->d[di->dst.xn] = (registers->d[di->dst.xn] & 0xffff0000) | (value & 0xffff);
        } else if (di->size == IS_BYTE) {
            setNZ(registers, (int32_t)(int8_t)(value & 0xff));
            registers->d[di->dst.xn] = (registers->d[di->dst.xn] & 0xffffff00) | (value & 0xff);
        }
    }
    else if (isTargetAddressRegister(di)) {
        cycleCount = 0;
        if (di->size == IS_LONG) {
            registers->a[di->dst.xn] = value;
        } else if (di->size == IS_WORD) {
            registers->a[di->dst.xn] = (int32_t)(int16_t)value;
        }
    }
    else if ((di->dst.mode == AM_ADDR_DISP) || (di->dst.mode == AM_ADDRESS) || (di->dst.mode == AM_ADDRESS_POST_INC)) {
        //printf("Store %x in $%06x\n", value, di->dst.address);
        preDecrement(di, registers, &di->dst);
        cycleCount = 4;
        if (di->size == IS_BYTE) {
            setNZ(registers, (int32_t)(int8_t)(value & 0xff));
            rwFunc->wb(readWriteUserdata, di->dst.address, value);
        } else if (di->size == IS_WORD) {
            setNZ(registers, (int32_t)(int16_t)(value & 0xffff));
            rwFunc->ww(readWriteUserdata, di->dst.address, value);
        } else if (di->size == IS_LONG) {
            setNZ(registers, (int32_t)value);
            rwFunc->ww(readWriteUserdata, di->dst.address, value >> 16);
            rwFunc->ww(readWriteUserdata, di->dst.address + 2, (uint16_t)value);
            cycleCount += 4;
        }
        postIncrement(di, registers, &di->dst);
    } else if (di->dst.mode == AM_EXT) {                
        if (di->dst.xn == AM_EXT_ABS_LONG) {
            if (di->size == IS_LONG) {
                rwFunc->ww(readWriteUserdata, di->dst.address, value >> 16);
                rwFunc->ww(readWriteUserdata, di->dst.address + 2, (uint16_t)value);
                cycleCount = 8;
            } else if (di->size == IS_WORD) {
                rwFunc->ww(readWriteUserdata, di->dst.address, (uint16_t)value);
                cycleCount = 4;
            } else {
                rwFunc->wb(readWriteUserdata, di->dst.address, (uint8_t)value);
                cycleCount = 4;
            }
        }
    }
    return cycleCount;
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
    setFlag(regs, SR_FLAGS_X, carry);
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
    dst = di->aluFunc(src,dst, di->size, registers);
    return cycleCount;
}

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

static int executeRts(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint32_t address = (uint32_t)rwFunc->rw(readWriteUserdata, registers->a[7]) << 16;
    address |= (uint32_t)rwFunc->rw(readWriteUserdata, registers->a[7]+2);
    registers->a[7] += 4;
    registers->pc = address;
    return 12;    
}

static int executeBranch(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    int cycles = 6;
    bool shouldBranch = true;
    if (di->condition == 1) {
        uint32_t address = registers->pc;
        registers->a[7] -= 4;
        rwFunc->ww(readWriteUserdata, registers->a[7], (uint16_t)(address >> 16));
        rwFunc->ww(readWriteUserdata, registers->a[7]+2, (uint16_t)address); 
        cycles += 8;
    } else if (di->condition == 4) { // BCC
        shouldBranch = !getFlag(registers, SR_FLAGS_C);
    } else if (di->condition == 5) { // BCS
        shouldBranch = getFlag(registers, SR_FLAGS_C);
    } else if (di->condition == 6) { // BNE
        shouldBranch = !getFlag(registers, SR_FLAGS_Z);
    } else if (di->condition == 7) { // BEQ
        shouldBranch = getFlag(registers, SR_FLAGS_Z);
    }

    if (shouldBranch) {
        registers->pc = align24(registers->pc + di->displacement);
    } else {
        return 4;
    }
    return cycles;  // We already counted the opcode fetch = 4 cycles
}

static char *mn_add = "ADD";
static char *mn_addx = "ADDX";
static char *mn_addq = "ADDQ";
static char *mn_cmp = "CMP";
static char *mn_subq = "SUBQ";
static char *mn_rts = "RTS";
static char *mn_lea = "LEA";
static char *mn_move = "MOVE";
static char *mn_moveq = "MOVEQ";
static char *mn_unknown = "???";
static char *mn_condition[] = {
    "BRA", "BSR", "BHI", "BLS", "BCC", "BCS", "BNE", "BEQ", "BVC", "BVS", "BPL", "BMI", "BGE", "BLT", "BGT", "BLE"};
static char *mn_btst = "BTST";

int decode(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata, ExecFunc *execFunc) {
    ReadWordFunc readWordFunc = rwFunc->rw;
    uint16_t cycles = 0;
    uint16_t opcode = readWordFunc(readWriteUserdata, registers->pc);
    increasePc(registers);
    cycles += 4;    

    di->mnemonic = mn_unknown;
    *execFunc = 0;

    uint16_t family = opcode & 0xf000;
    if (family == 0) {
        uint16_t dstMode = (opcode >> 6) & 7;
        uint16_t dstReg = (opcode >> 9) & 7;   
        uint16_t srcMode = (opcode >> 3) & 7;
        uint16_t srcReg = opcode & 7;
        if ((dstReg == 4) && (dstMode == 0)) { // BTST #n,<ea>
            *execFunc = executeBtst;
            di->family = IF_MOVE;
            di->mnemonic = mn_btst;            
            uint16_t size = (srcMode == AM_DREG) ? IS_LONG : IS_BYTE;
            di->size = size;            
            di->src.mode = AM_EXT;
            di->src.xn = AM_EXT_IMMEDIATE;
            int eaCycles = getEffectiveAddress(registers, di->src.mode, di->src.xn, size, &di->src, readWordFunc, readWriteUserdata);
            if (eaCycles < 0) {
                return 0;
            }
            cycles += eaCycles;
            eaCycles = getEffectiveAddress(registers, srcMode, srcReg, size, &di->dst, readWordFunc, readWriteUserdata);
            if (eaCycles < 0) {
                return 0;
            }
            cycles += eaCycles;
        }

    } else if (family < 0x4000) {    
        *execFunc = executeMove;
        di->family = IF_MOVE;
        di->mnemonic = mn_move;
        //  move.x
        uint16_t size = (family >> 12);
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
        cycles += eaCycles;

        eaCycles = getEffectiveAddress(registers, dstMode, dstReg, size, &di->dst, readWordFunc, readWriteUserdata);
        if (eaCycles < 0) {
            return 0;
        }
        cycles += eaCycles;
        return cycles;
    } else if (family == 0xb000) {
        di->family = IF_MOVE;
        *execFunc = executeCmp;
        uint16_t mode = (opcode >> 3) & 7;
        uint16_t srcReg = opcode & 7;
        uint16_t opMode = (opcode >> 6) & 7;
        uint16_t dstReg = (opcode >> 9) & 7;  
        if ((opMode == 0) || (opMode == 1) || (opMode == 2)) {
            // CMP.x
            di->mnemonic = mn_cmp;
            uint16_t size = opMode & 3;
            di->size = size;
            EffectiveAddress ea;
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
        } else {
            return 0;
        }

    } else if (family == 0xd000) { // ADD
        di->family = IF_MOVE;
        di->mnemonic = mn_add;
        *execFunc = executeAlu;
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
            *execFunc = executeAlu;
            if (isSub) {
                di->mnemonic = mn_subq;
                di->aluFunc = aluSub;
            } else {
                di->mnemonic = mn_addq;
                di->aluFunc = aluAdd;
            }
            cycles += eaCycles;
        }
    } else if (family == 0x7000) { // MOVEQ
        *execFunc = executeMove;
        di->family = IF_MOVE;
        di->mnemonic = mn_moveq;
        uint16_t dstReg = (opcode >> 9) & 7;                
        di->size = IS_LONG;
        di->src.mode = AM_EXT;
        di->src.xn = AM_EXT_IMMEDIATE;
        di->src.immediate = (int32_t)(int8_t)(opcode);
        int eaCycles = getEffectiveAddress(registers, AM_DREG , dstReg, IS_LONG, &di->dst, readWordFunc, readWriteUserdata);
        if (eaCycles < 0) {
            return 0;
        }
        cycles += eaCycles;
        return cycles;
    } else if ((opcode & 0xf1c0) == 0x41c0) { // LEA
        *execFunc = executeLea;
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

    } else if (family == 0x6000) { // BRA, BSR, Bxx
        uint16_t condition = (opcode >> 8) & 15;
        di->condition = condition;
        di->mnemonic = mn_condition[condition];        
        di->family = IF_BRANCH;
        *execFunc = executeBranch;
        uint8_t displacement = opcode & 0xff;
        if (displacement == 0) {
            di->displacement = (int32_t)(int16_t)readWordFunc(readWriteUserdata, registers->pc)-2; 
            // !! Displacement is taken from the PC after the opcode word not after the extension word
            di->size = IS_WORD;
            increasePc(registers);
            //cycles += 4; Don't update cycles here because branch is annoying
        } else {
            di->displacement = (int8_t)displacement;
            di->size = IS_BYTE;
        }
    } else if (opcode == 0x4e75) {
        di->mnemonic = mn_rts;
        di->family = IF_IMPLIED;
        *execFunc = executeRts;
    }
    return cycles;
}
