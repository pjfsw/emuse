#pragma once

#include "spi.h"
#include "booleanfunc.h"

typedef struct {
    Spi *spi;
} Mmc;

void mmcInit(Mmc *mmc, Spi *spi);

void mmcClock(void *userdata, int clocks);

uint8_t mmcGetMiso(Mmc *mmc);