#include "m68k.h"
#include "decode.h"

void pushWord(M68kRegisters *regs, RwFunc *rw, void *readWriteUserdata, uint16_t word) {
    regs->a[7] = align24(regs->a[7] - 2);
    rw->ww(readWriteUserdata, regs->a[7], word);
}

void push(M68k *cpu, uint16_t word) {
    pushWord(&cpu->registers, &cpu->rwFunc, cpu->readWriteUserdata, word);
}

uint16_t pop(M68kRegisters *regs, RwFunc *rw, void *readWriteUserdata) {
    uint16_t value = rw->rw(readWriteUserdata, regs->a[7]);
    regs->a[7] = align24(regs->a[7] + 2);
    return value;
}