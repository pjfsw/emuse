#include "movem.h"
#include "sourcedest.h"

bool isMovemToRegister(uint16_t opcode) {
    return (opcode >> 10) & 1;
}

static int copyToRegister(
    uint32_t *reg, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    uint32_t value;
    int cycles = readSource(di, registers, &di->src, rwFunc, readWriteUserdata, &value);
    if (di->size == IS_WORD) {
        uint16_t w = value & 0xffff;
        *reg = (uint32_t)(int32_t)(int16_t)w;
    } else {
        *reg = value;
    }
    return cycles;
}

static int copyFromRegister(
    uint32_t *reg, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    
    return  writeDestNoFlags(di, registers, rwFunc, readWriteUserdata, *reg);
}

/*
 * Control addressing modes: (An), d16(An), d8(An,Xn), abs.w, abs.l, and
 * for memory to registers also d16(PC) and d8(PC,Xn).
 */
static bool isMovemControlMode(EffectiveAddress *src, EffectiveAddress *dst, bool toRegister) {
    EffectiveAddress *ea = toRegister ? src : dst;
    if ((ea->mode == AM_ADDRESS) || (ea->mode == AM_ADDR_DISP) || (ea->mode == AM_ADDR_INDEX)) {
        return true;
    }
    if (ea->mode == AM_EXT) {
        if ((ea->xn == AM_EXT_ABS_SHORT) || (ea->xn == AM_EXT_ABS_LONG)) {
            return true;
        }
        if (toRegister && ((ea->xn == AM_EXT_PC_DISP) || (ea->xn == AM_EXT_PC_INDEX))) {
            return true;
        }
    }
    return false;
}

/*
 * MOVEM with a control addressing mode: the registers are transferred
 * in the order d0-d7, a0-a7 to/from increasing addresses, starting at
 * the effective address. The address register is not updated.
 */
static int executeMovemControl(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    bool toRegister = isMovemToRegister(di->opcode);
    uint32_t mask = toRegister ? di->dst.xn : di->src.xn;
    uint32_t address = toRegister ? di->src.address : di->dst.address;
    int cycles = toRegister ? 4 : 0;    // Memory to registers takes 4 cycles more

    for (int i = 0; i < 16; i++) {
        if (!(mask & (1 << i))) {
            continue;
        }
        uint32_t *reg = (i < 8) ? &registers->d[i] : &registers->a[i - 8];
        if (di->size == IS_LONG) {
            if (toRegister) {
                *reg = ((uint32_t)rwFunc->rw(readWriteUserdata, address) << 16) |
                       (uint32_t)rwFunc->rw(readWriteUserdata, address + 2);
            } else {
                rwFunc->ww(readWriteUserdata, address, *reg >> 16);
                rwFunc->ww(readWriteUserdata, address + 2, (uint16_t)*reg);
            }
            address += 4;
            cycles += 8;
        } else {
            if (toRegister) {
                // Words are sign extended to the whole register, also for data registers
                *reg = (uint32_t)(int32_t)(int16_t)rwFunc->rw(readWriteUserdata, address);
            } else {
                rwFunc->ww(readWriteUserdata, address, (uint16_t)*reg);
            }
            address += 2;
            cycles += 4;
        }
        address = align24(address);
    }
    return cycles;
}

static int executeMovem(DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    int cycles = 0;
    if (isMovemToRegister(di->opcode) && (di->src.mode == AM_ADDRESS_POST_INC)) {
        for (int i = 0; i < 8; i++) {
            int n = (1 << i);
            if (di->dst.xn & n) {
                cycles += copyToRegister(&registers->d[i], di, registers, rwFunc, readWriteUserdata);
            }
        }
        for (int i = 0; i < 8; i++) {
            int n = (1 << (i+8));
            if (di->dst.xn & n) {
                cycles += copyToRegister(&registers->a[i], di, registers, rwFunc, readWriteUserdata);
            }
        }
        return cycles;
    } else if (!isMovemToRegister(di->opcode) && (di->dst.mode == AM_ADDRESS_PRE_DEC)) {
        for (int i = 0; i < 8; i++) {
            int n = (1 << i);
            if (di->src.xn & n) {
                cycles += copyFromRegister(&registers->a[7-i], di, registers, rwFunc, readWriteUserdata);
            }
        }
        for (int i = 0; i < 8; i++) {
            int n = (1 << (i+8));
            if (di->src.xn & n) {
                cycles += copyFromRegister(&registers->d[7-i], di, registers, rwFunc, readWriteUserdata);
            }
        }
        return cycles;
    } else if (isMovemControlMode(&di->src, &di->dst, isMovemToRegister(di->opcode))) {
        return executeMovemControl(di, registers, rwFunc, readWriteUserdata);
    } else {
        return -1;
    }
}

int decodeMovem(uint16_t opcode, DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata) {
    ReadWordFunc readWordFunc = rwFunc->rw;
    di->execFunc = executeMovem;
    di->mnemonic = "MOVEM";
    uint16_t eaMode = (opcode >> 3) & 7;
    uint16_t eaReg = opcode & 7;
    uint16_t size = (opcode >> 6) & 1;
    if (size) {
        di->size = IS_LONG;
    } else {
        di->size = IS_WORD;
    } 
    EffectiveAddress *ea;
    uint32_t *mask;
    if (isMovemToRegister(opcode)) {
        ea = &di->src;
        mask = &di->dst.xn;
    } else {
        ea = &di->dst;
        mask = &di->src.xn;
    }

    // The register mask comes before the extension words of the effective address
    *mask = (uint32_t)readWordFunc(readWriteUserdata, registers->pc);
    increasePc(registers);

    int eaCycles = getEffectiveAddress(registers, eaMode, eaReg, di->size, ea, readWordFunc, readWriteUserdata);
    if (eaCycles < 0) {
        return -1;
    }
    return eaCycles+8;
}   
