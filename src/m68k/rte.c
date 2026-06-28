#include "decode.h"
#include "pushpop.h"

static int executeRte(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    if (!getFlag(registers, SR_FLAGS_S)) {
        // Add exception later
        return -1;
    }

    registers->sr = pop(registers, rwFunc, readWriteUserdata);
    uint32_t pc =  (uint32_t)pop(registers, rwFunc, readWriteUserdata) << 16;
    pc |= (uint32_t)pop(registers, rwFunc, readWriteUserdata);

    if (!getFlag(registers, SR_FLAGS_S)) {
        registers->ssp = registers->a[7];
        registers->a[7] = registers->usp;
    }
    registers->pc = align24(pc);
    return 16; // 4 cycles already accounted for in instruction fetch    
}

int decodeRte(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    di->mnemonic = "RTE";
    di->execFunc = executeRte;
    return 0;
}
