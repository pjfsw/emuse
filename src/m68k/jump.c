#include "jump.h"
#include <stdio.h>

static int executeJsr(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint32_t address = registers->pc;
    registers->a[7] -= 4;
    rwFunc->ww(readWriteUserdata, registers->a[7], (uint16_t)(address >> 16));
    rwFunc->ww(readWriteUserdata, registers->a[7]+2, (uint16_t)address); 
    registers->pc = align24(di->dst.address);    
    //printf("JSR %06x\n", registers->pc);
    return 12;
}

static int executeJmp(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    registers->pc = align24(di->dst.address);    
    //printf("JMP %06x\n", registers->pc);
    return 4;
}

static int decodeJmpJsr(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    ReadWordFunc readWordFunc = rwFunc->rw;
    di->size = IS_LONG;

    uint16_t mode = (opcode >> 3) & 7;
    uint16_t xn = opcode & 7;

    int eaCycles = getEffectiveAddress(registers, mode, xn, IS_LONG, &di->dst, readWordFunc, readWriteUserdata);
    if (eaCycles < 0) {
        return -1;
    }
    return eaCycles;
}

int decodeJsr(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    di->execFunc = executeJsr;
    di->mnemonic = "JSR";
    return decodeJmpJsr(opcode, di, registers, rwFunc, readWriteUserdata);
}

int decodeJmp(
    uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    di->execFunc = executeJmp;
    di->mnemonic = "JMP";
    return decodeJmpJsr(opcode, di, registers, rwFunc, readWriteUserdata);
}
