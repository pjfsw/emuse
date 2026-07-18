#include "mmc.h"

void mmcInit(Mmc *mmc) {

}

void mmcClock(void *userdata, int clocks) {
    Mmc *mmc = (Mmc*)userdata;
    mmc->spi.miso = 1;
}

uint8_t mmcGetMiso(Mmc *mmc) {
    return mmc->spi.miso & 1;
}