#pragma once

#include "decode.h"

// ILLEGAL ($4afc): illegal instruction exception
int decodeIllegal(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata);

// $axxx: line-A emulator exception
int decodeLineA(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata);

// $fxxx: line-F emulator exception
int decodeLineF(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata);
