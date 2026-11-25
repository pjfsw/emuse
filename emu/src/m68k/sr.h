#pragma once

#include "decode.h"

// Write the status register: unimplemented bits read as zero, and a7
// is switched to the user stack pointer when leaving supervisor mode.
void setSr(M68kRegisters *registers, uint16_t value);

// ORI/ANDI/EORI #imm,CCR
int decodeOriCcr(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata);
int decodeAndiCcr(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata);
int decodeEoriCcr(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata);

// ORI/ANDI/EORI #imm,SR (privileged)
int decodeOriSr(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata);
int decodeAndiSr(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata);
int decodeEoriSr(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata);
