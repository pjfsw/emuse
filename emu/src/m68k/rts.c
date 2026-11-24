#include "decode.h"

static int executeRts(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint32_t address = (uint32_t)rwFunc->rw(readWriteUserdata, registers->a[7]) << 16;
    address |= (uint32_t)rwFunc->rw(readWriteUserdata, registers->a[7]+2);
    registers->a[7] += 4;
    registers->pc = address;
    return 12;    
}

int decodeRts(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    di->mnemonic = "RTS";
    di->execFunc = executeRts;
    return 0;
}
