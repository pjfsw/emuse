#include "nop.h"

static int executeNop(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    return 0; // 4 cycles, already accounted for in instruction fetch
}

int decodeNop(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    di->mnemonic = "NOP";
    di->execFunc = executeNop;
    return 0;
}
