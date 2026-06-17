#include "sourcedest.h"

static void preDecrement(DecodedInstruction *di, M68kRegisters *registers, EffectiveAddress *ea) {
    if (ea->mode != AM_ADDRESS_PRE_DEC) {
        return;
    }
}

static void postIncrement(DecodedInstruction *di, M68kRegisters *registers, EffectiveAddress *ea) {
    if (ea->mode != AM_ADDRESS_POST_INC) {
        return;
    }
    int size = 0;
    if (di->size == IS_LONG) {
        size = 4;
    } else if (di->size == IS_WORD) {
        size = 2;
    } else {
        if (ea->xn == 7) {
            // Stack special case
            size = 2;
        } else {
            size = 1;
        }
    }
    registers->a[ea->xn] = align24(registers->a[ea->xn] + size);
}

int readSource(
    DecodedInstruction *di, M68kRegisters *registers, EffectiveAddress *ea, RwFunc *rwFunc, void *readWriteUserdata, uint32_t *value) {
    int cycleCount = -1;
    if (ea->mode == AM_EXT) {
        if (ea->xn == AM_EXT_IMMEDIATE) {
            *value = ea->immediate;
            cycleCount = 0;
        } else if (ea->xn == AM_EXT_ABS_LONG) {
            if (di->size == IS_LONG) {
                *value = (uint32_t)rwFunc->rw(readWriteUserdata, ea->address) << 16;
                *value |= (uint32_t)rwFunc->rw(readWriteUserdata, ea->address + 2);
                cycleCount = 8;
            } else if (di->size == IS_WORD) {
                *value = rwFunc->rw(readWriteUserdata, ea->address);
                cycleCount = 4;
            } else {
                *value = rwFunc->rb(readWriteUserdata, ea->address);
                cycleCount = 4;
            }
        }
    } else if (ea->mode == AM_DREG) {
        *value = registers->d[ea->xn];
        cycleCount = 0;
    } else if (ea->mode == AM_AREG) {
        *value = registers->a[ea->xn];
        cycleCount = 0;
    } else if ((ea->mode == AM_ADDRESS) || (ea->mode == AM_ADDR_DISP) || (ea->mode == AM_ADDRESS_POST_INC)) {
        preDecrement(di, registers, ea);
//        printf("Read $%06x\n", di->src.address);
        cycleCount = 4;
        if (di->size == IS_BYTE) {
            *value = (uint32_t)rwFunc->rb(readWriteUserdata, ea->address);
        } else if (di->size == IS_WORD) {
            *value = (uint32_t)rwFunc->rw(readWriteUserdata, ea->address);
        } else if (di->size == IS_LONG) {
            *value = (uint32_t)rwFunc->rw(readWriteUserdata, ea->address) << 16;
            *value |= (uint32_t)rwFunc->rw(readWriteUserdata, ea->address + 2);
            cycleCount += 4;
        }
        postIncrement(di, registers, ea);

    }
    return cycleCount;
}

int writeDest(
    DecodedInstruction *di, M68kRegisters *registers, RwFunc *rwFunc, void *readWriteUserdata, uint32_t value) {
    uint32_t cycleCount = -1;

    if ((di->dst.mode == AM_DREG)) {
        cycleCount = 0;
        if (di->size == IS_LONG) {
            setNZ(registers, (int32_t)value);
            registers->d[di->dst.xn] = value;
        } else if (di->size == IS_WORD) {
            setNZ(registers, (int32_t)(int16_t)(value & 0xffff));
            registers->d[di->dst.xn] = (registers->d[di->dst.xn] & 0xffff0000) | (value & 0xffff);
        } else if (di->size == IS_BYTE) {
            setNZ(registers, (int32_t)(int8_t)(value & 0xff));
            registers->d[di->dst.xn] = (registers->d[di->dst.xn] & 0xffffff00) | (value & 0xff);
        }
    }
    else if (isTargetAddressRegister(di)) {
        cycleCount = 0;
        if (di->size == IS_LONG) {
            registers->a[di->dst.xn] = value;
        } else if (di->size == IS_WORD) {
            registers->a[di->dst.xn] = (int32_t)(int16_t)value;
        }
    }
    else if ((di->dst.mode == AM_ADDR_DISP) || (di->dst.mode == AM_ADDRESS) || (di->dst.mode == AM_ADDRESS_POST_INC)) {
        //printf("Store %x in $%06x\n", value, di->dst.address);
        preDecrement(di, registers, &di->dst);
        cycleCount = 4;
        if (di->size == IS_BYTE) {
            setNZ(registers, (int32_t)(int8_t)(value & 0xff));
            rwFunc->wb(readWriteUserdata, di->dst.address, value);
        } else if (di->size == IS_WORD) {
            setNZ(registers, (int32_t)(int16_t)(value & 0xffff));
            rwFunc->ww(readWriteUserdata, di->dst.address, value);
        } else if (di->size == IS_LONG) {
            setNZ(registers, (int32_t)value);
            rwFunc->ww(readWriteUserdata, di->dst.address, value >> 16);
            rwFunc->ww(readWriteUserdata, di->dst.address + 2, (uint16_t)value);
            cycleCount += 4;
        }
        postIncrement(di, registers, &di->dst);
    } else if (di->dst.mode == AM_EXT) {                
        if (di->dst.xn == AM_EXT_ABS_LONG) {
            if (di->size == IS_LONG) {
                rwFunc->ww(readWriteUserdata, di->dst.address, value >> 16);
                rwFunc->ww(readWriteUserdata, di->dst.address + 2, (uint16_t)value);
                cycleCount = 8;
            } else if (di->size == IS_WORD) {
                rwFunc->ww(readWriteUserdata, di->dst.address, (uint16_t)value);
                cycleCount = 4;
            } else {
                rwFunc->wb(readWriteUserdata, di->dst.address, (uint8_t)value);
                cycleCount = 4;
            }
        }
    }
    return cycleCount;
}
