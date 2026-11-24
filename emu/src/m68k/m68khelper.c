#include "decode.h"

inline uint32_t align24(uint32_t address) {
    return address & 0xffffff;
}

inline void increasePc(M68kRegisters *registers) {
    registers->pc = align24(registers->pc + 2);
}

inline bool isTargetAddressRegister(DecodedInstruction *di) {
    return di->dst.mode == AM_AREG;
}
