#pragma once

#include "decode.h"

int decodeBranch(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata);
