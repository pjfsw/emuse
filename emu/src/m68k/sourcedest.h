#pragma once

#include "decode.h"

int readSource(DecodedInstruction *di, M68kRegisters *registers, EffectiveAddress *ea, RwFunc *rwFunc,
    void *readWriteUserdata, uint32_t *value);

int writeDest(
    DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata, uint32_t value);

int writeDestNoFlags(
    DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata, uint32_t value);    