#pragma once

#include "decode.h"

bool isMovemToRegister(uint16_t opcode);

int decodeMovem(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata);