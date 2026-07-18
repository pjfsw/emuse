#pragma once

#include <stdint.h>
#include "mmc.h"

typedef struct {
    Mmc *mmc;
} Ireg;

void iregInit(Ireg *ireg, Mmc *mmc);

uint8_t iregReadByte(void *userdata, uint32_t address);

uint16_t iregReadWord(void *userdata, uint32_t address);