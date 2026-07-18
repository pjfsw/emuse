#pragma once

#include "spi.h"

typedef struct {
    Spi spi;
} Mmc;

void mmcInit(Mmc *mmc);

void mmcClock(void *userdata, int clocks);

uint8_t mmcGetMiso(Mmc *mmc);