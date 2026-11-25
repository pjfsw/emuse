#include "sr.h"

// Bits implemented in the 68000 status register: T, S, I2-I0, X, N, Z, V, C
#define SR_IMPLEMENTED_MASK 0xa71f
#define CCR_MASK            0x001f

void setSr(M68kRegisters *registers, uint16_t value) {
    bool wasSupervisor = getFlag(registers, SR_FLAGS_S);
    registers->sr = value & SR_IMPLEMENTED_MASK;
    if (wasSupervisor && !getFlag(registers, SR_FLAGS_S)) {
        registers->ssp = registers->a[7];
        registers->a[7] = registers->usp;
    }
}

// Apply ORI (opcode bits 11-8 = 0000), ANDI (0010) or EORI (1010)
static uint16_t applyLogicOp(uint16_t opcode, uint16_t value, uint16_t immediate) {
    switch (opcode & 0x0f00) {
        case 0x0000:
            return value | immediate;
        case 0x0200:
            return value & immediate;
        default:
            return value ^ immediate;
    }
}

static int executeToCcr(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint16_t ccr = applyLogicOp(di->opcode, registers->sr & CCR_MASK, di->src.immediate) & CCR_MASK;
    registers->sr = (registers->sr & ~CCR_MASK) | ccr;
    return 12; // 20 cycles in total
}

static int executeToSr(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    if (!getFlag(registers, SR_FLAGS_S)) {
        // TODO!! Throw privilege violation exception (same as MOVE to SR)
        return -1;
    }
    setSr(registers, applyLogicOp(di->opcode, registers->sr, di->src.immediate));
    return 12; // 20 cycles in total
}

static int decodeImmediateToStatus(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc,
    void *readWriteUserdata, InstructionSize size, ExecFunc execFunc) {
    di->execFunc = execFunc;
    di->size = size;
    return getEffectiveAddress(registers, AM_EXT, AM_EXT_IMMEDIATE, size, &di->src, rwFunc->rw, readWriteUserdata);
}

int decodeOriCcr(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    di->mnemonic = "ORI";
    return decodeImmediateToStatus(di, registers, rwFunc, readWriteUserdata, IS_BYTE, executeToCcr);
}

int decodeAndiCcr(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    di->mnemonic = "ANDI";
    return decodeImmediateToStatus(di, registers, rwFunc, readWriteUserdata, IS_BYTE, executeToCcr);
}

int decodeEoriCcr(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    di->mnemonic = "EORI";
    return decodeImmediateToStatus(di, registers, rwFunc, readWriteUserdata, IS_BYTE, executeToCcr);
}

int decodeOriSr(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    di->mnemonic = "ORI";
    return decodeImmediateToStatus(di, registers, rwFunc, readWriteUserdata, IS_WORD, executeToSr);
}

int decodeAndiSr(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    di->mnemonic = "ANDI";
    return decodeImmediateToStatus(di, registers, rwFunc, readWriteUserdata, IS_WORD, executeToSr);
}

int decodeEoriSr(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    di->mnemonic = "EORI";
    return decodeImmediateToStatus(di, registers, rwFunc, readWriteUserdata, IS_WORD, executeToSr);
}
