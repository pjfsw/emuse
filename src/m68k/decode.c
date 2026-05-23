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
    } else if (mode == AM_ADDRESS) { // (a0)
        ea->address = registers->a[reg];
        return 0;        
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
        }
    }
    return -1;
}

static void setFlag(M68kRegisters *registers, uint16_t flag, uint16_t value) {
    registers->sr = (registers->sr & (0xffff-flag)) | (value * flag);
}

static void setNZ(M68kRegisters *registers, int32_t value) {
    setFlag(registers, SR_FLAGS_N, value < 0);
    setFlag(registers, SR_FLAGS_Z, value == 0);
}

static int executeLea(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    registers->a[di->dst.xn] = di->src.address;
    return 0;
}

static int readSource(
    DecodedInstruction *di, M68kRegisters *registers, EffectiveAddress *ea, RwFunc *rwFunc, void *readWriteUserdata, uint32_t *value) {
    uint32_t cycleCount = 0;
    if (ea->mode == AM_EXT) {
        if (ea->xn == AM_EXT_IMMEDIATE) {
            *value = ea->immediate;
        } else if (ea->xn == AM_EXT_ABS_LONG) {
            if (di->size == IS_LONG) {
                *value = (uint32_t)rwFunc->rw(readWriteUserdata, ea->address) << 16;
                *value |= (uint32_t)rwFunc->rw(readWriteUserdata, ea->address + 2);
                cycleCount = 8;
            } else {
                *value = rwFunc->rw(readWriteUserdata, ea->address);
                cycleCount = 4;
            }
        }
    } else if (ea->mode == AM_DREG) {
        *value = registers->d[ea->xn];
    } else if (ea->mode == AM_AREG) {
        *value = registers->a[ea->xn];
    }
    return cycleCount;
}

static int writeDest(
    DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata, uint32_t value) {
    uint32_t cycleCount = 0;

    if ((di->dst.mode == AM_DREG)) {
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
    else if (di->dst.mode == AM_AREG) {
        if (di->size == IS_LONG) {
            registers->a[di->dst.xn] = value;
        } else if (di->size == IS_WORD) {
            registers->a[di->dst.xn] = (int32_t)(int16_t)value;
        }
    }
    else if (di->dst.mode == AM_ADDRESS) {
        printf("Store %x in $%06x\n", value, di->dst.address);
        cycleCount += 4;
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
    }
    return cycleCount;
}

static uint32_t aluAdd(uint32_t a, uint32_t b) {
    return a + b;
}

static int executeAlu(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint32_t src;
    uint32_t dst;
    int cycleCount = readSource(di, registers, &di->src, rwFunc, readWriteUserdata, &src);
    cycleCount += readSource(di, registers, &di->dst, rwFunc, readWriteUserdata, &dst);
    dst = di->aluFunc(src,dst);
    cycleCount += writeDest(di, registers, rwFunc, readWriteUserdata, dst);
    return cycleCount;
}

static int executeMove(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint32_t value;
    int cycleCount = readSource(di, registers, &di->src, rwFunc, readWriteUserdata, &value);
    // todo clear C,V and leave X unchanged
    cycleCount += writeDest(di, registers, rwFunc, readWriteUserdata, value);
    return cycleCount;
}

int executeRts(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint32_t address = (uint32_t)rwFunc->rw(readWriteUserdata, registers->a[7]) << 16;
    address |= (uint32_t)rwFunc->rw(readWriteUserdata, registers->a[7]+2);
    registers->a[7] += 4;
    registers->pc = address;
    return 12;    
}

int executeBranch(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    int cycles = 6;
    bool shouldBranch = true;
    if (di->condition == 1) {
        uint32_t address = registers->pc;
        registers->a[7] -= 4;
        rwFunc->ww(readWriteUserdata, registers->a[7], (uint16_t)(address >> 16));
        rwFunc->ww(readWriteUserdata, registers->a[7]+2, (uint16_t)address); 
        cycles += 8;
    }
    // TODO condition
    /*printf("Current pc %06x, displacement %04x, new pc %06x\n",
            registers->pc,
            di->displacement,
            registers->pc+di->displacement);*/
    if (shouldBranch) {
        registers->pc = align24(registers->pc + di->displacement);
    }
    return cycles;  // We already counted the opcode fetch = 4 cycles
}

static char *mn_add = "ADD";
static char *mn_rts = "RTS";
static char *mn_lea = "LEA";
static char *mn_move = "MOVE";
static char *mn_moveq = "MOVEQ";
static char *mn_unknown = "???";
static char *mn_condition[] = {
    "BRA", "BSR", "BHI", "BLS", "BCC", "BCS", "BNE", "BEQ", "BVC", "BVS", "BPL", "BMI", "BGE", "BLT", "BGT", "BLE"};


int decode(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata, ExecFunc *execFunc) {
    ReadWordFunc readWordFunc = rwFunc->rw;
    uint16_t cycles = 0;
    uint16_t opcode = readWordFunc(readWriteUserdata, registers->pc);
    increasePc(registers);
    cycles += 4;    

    di->mnemonic = mn_unknown;
    *execFunc = 0;

    uint16_t family = opcode & 0xf000;
    if ((family > 0) && (family < 0x4000)) {    
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
