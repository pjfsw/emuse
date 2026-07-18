#include "input_reg.h"

void iregInit(Ireg *ireg, Mmc *mmc) {
    ireg->mmc = mmc;
}

static uint8_t readByte(Ireg *ireg) {
    return (mmcGetMiso(ireg->mmc)) << 7;
}

uint8_t iregReadByte(void *userdata, uint32_t address) {
    if ((address & 1) == 0) {
        return 0xff;
    }
    return readByte((Ireg *)userdata);
}

uint16_t iregReadWord(void *userdata, uint32_t address) {
    return (uint16_t)readByte((Ireg *)userdata);
}