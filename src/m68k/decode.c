#include "decode.h"

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

    if (mode == AM_DREG) {
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
        }
    }
    return -1;
}

static int executeMove(DecodedInstruction *di, M68kRegisters *registers, ReadByteFunc readByteFunc, ReadWordFunc readWordFunc,
    void *readWriteUserdata) {
        int cycleCount = 0;
        uint32_t value = 0;
        if (di->src.mode == AM_EXT) {
            if (di->src.xn == AM_EXT_IMMEDIATE) {
                value = di->src.immediate;
            }
        } else if (di->src.mode == AM_DREG) {
            value = registers->d[di->src.xn];
        }
        if (di->dst.mode == AM_DREG) {
            if (di->size == IS_LONG) {
                registers->d[di->dst.xn] = value;
            } else if (di->size == IS_WORD) {
                registers->d[di->dst.xn] = (registers->d[di->dst.xn] & 0xffff0000) | (value & 0xffff);
            } else if (di->size == IS_BYTE) {
                registers->d[di->dst.xn] = (registers->d[di->dst.xn] & 0xffffff00) | (value & 0xff);
            }
        }
        return cycleCount;
    }

static char *mn_move = "move";
static char *mn_unknown = "??";
int decode(DecodedInstruction *di, M68kRegisters *registers, ReadByteFunc readByteFunc, ReadWordFunc readWordFunc,
    void *readWriteUserdata) {
    uint16_t cycles = 0;
    uint16_t opcode = readWordFunc(readWriteUserdata, registers->pc);
    increasePc(registers);
    cycles += 4;    

    di->mnemonic = mn_unknown;
    uint16_t family = opcode & 0xf000;
    if ((family > 0) && (family < 0x4000)) {    
        di->mnemonic = mn_move;
        //  move.x
        uint16_t size = (family >> 12);
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

        int execCycles = executeMove(di, registers, readByteFunc, readWordFunc, readWriteUserdata);
        if (execCycles < 0) {
            return 0;
        }
        cycles += eaCycles;

    }
    return cycles;
}
