#include "movem.h"
#include "sourcedest.h"

bool isMovemToRegister(uint16_t opcode) {
    return (opcode >> 10) & 1;
}

static int copyToRegister(
    uint32_t *reg, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint32_t value;
    int cycles = readSource(di, registers, &di->src, rwFunc, readWriteUserdata, &value);
    if (di->size == IS_WORD) {
        uint16_t w = value & 0xffff;
        *reg = (uint32_t)(int32_t)(int16_t)w;
    } else {
        *reg = value;
    }
    return cycles;
}

static int copyFromRegister(
    uint32_t *reg, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    
    return  writeDestNoFlags(di, registers, rwFunc, readWriteUserdata, *reg);
}

static int executeMovem(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    int cycles = 0;
    if (isMovemToRegister(di->opcode) && (di->src.mode == AM_ADDRESS_POST_INC)) {
        for (int i = 0; i < 8; i++) {
            int n = (1 << i);
            if (di->dst.xn & n) {
                cycles += copyToRegister(&registers->d[i], di, registers, rwFunc, readWriteUserdata);
            }
        }
        for (int i = 0; i < 8; i++) {
            int n = (1 << (i+8));
            if (di->dst.xn & n) {
                cycles += copyToRegister(&registers->a[i], di, registers, rwFunc, readWriteUserdata);
            }
        }
        return cycles;
    } else if (!isMovemToRegister(di->opcode) && (di->dst.mode == AM_ADDRESS_PRE_DEC)) {
        for (int i = 0; i < 8; i++) {
            int n = (1 << i);
            if (di->src.xn & n) {
                cycles += copyFromRegister(&registers->a[7-i], di, registers, rwFunc, readWriteUserdata);
            }
        }
        for (int i = 0; i < 8; i++) {
            int n = (1 << (i+8));
            if (di->src.xn & n) {
                cycles += copyFromRegister(&registers->d[7-i], di, registers, rwFunc, readWriteUserdata);
            }
        }
        return cycles;
    } else {
        // Implement other modes here
        return -1;
    }
}

int decodeMovem(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    ReadWordFunc readWordFunc = rwFunc->rw;
    di->execFunc = executeMovem;
    di->mnemonic = "MOVEM";
    uint16_t eaMode = (opcode >> 3) & 7;
    uint16_t eaReg = opcode & 7;
    uint16_t size = (opcode >> 6) & 1;
    if (size) {
        di->size = IS_LONG;
    } else {
        di->size = IS_WORD;
    } 
    EffectiveAddress *ea;
    uint32_t *mask;
    if (isMovemToRegister(opcode)) {
        ea = &di->src;
        mask = &di->dst.xn;
    } else {
        ea = &di->dst;
        mask = &di->src.xn;
    }

    int eaCycles = getEffectiveAddress(registers, eaMode, eaReg, di->size, ea, readWordFunc, readWriteUserdata);
    if (eaCycles < 0) {
        return -1;
    }

    *mask = (uint32_t)readWordFunc(readWriteUserdata, registers->pc);
    increasePc(registers);
    return eaCycles+8;
}   
