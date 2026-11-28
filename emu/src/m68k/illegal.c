#include "illegal.h"

// The exception set by the decode function is raised by m68kClock
static int executeException(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    return 0;
}

static int decodeException(DecodedInstruction *di, char *mnemonic, uint8_t vector) {
    di->mnemonic = mnemonic;
    di->execFunc = executeException;
    di->exception = vector;
    return 0;
}

int decodeIllegal(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    return decodeException(di, "ILLEGAL", VECTOR_ILLEGAL_INSTRUCTION);
}

int decodeLineA(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    return decodeException(di, "LINE-A", VECTOR_LINE_A_EMULATOR);
}

int decodeLineF(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    return decodeException(di, "LINE-F", VECTOR_LINE_F_EMULATOR);
}
