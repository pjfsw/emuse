#pragma once

#include "m68k.h"

void push(M68k *cpu, uint16_t word);

uint16_t pop(M68kRegisters *regs, RwFunc *rw, void *readWriteUserdata);