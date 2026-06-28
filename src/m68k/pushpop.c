#include "m68k.h"
#include "decode.h"

void push(M68k *cpu, uint16_t word) {
    M68kRegisters *regs = &cpu->registers;
    RwFunc *rw = &cpu->rwFunc;    
    void *readWriteUserdata = cpu->readWriteUserdata;
    regs->a[7] = align24(regs->a[7] - 2);
    rw->ww(readWriteUserdata, regs->a[7], word);
}

uint16_t pop(M68kRegisters *regs, RwFunc *rw, void *readWriteUserdata) {
    uint16_t value = rw->rw(readWriteUserdata, regs->a[7]);
    regs->a[7] = align24(regs->a[7] + 2);
    return value;
}