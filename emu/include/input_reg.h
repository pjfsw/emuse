#pragma once

#include <stdint.h>
#include "mmc.h"
#include "snes_pad.h"

typedef struct {
    Mmc *mmc;
    SnesPad *snesPad;
} Ireg;

void iregInit(Ireg *ireg, Mmc *mmc, SnesPad *snesPad);

uint8_t iregReadByte(void *userdata, uint32_t address);

uint16_t iregReadWord(void *userdata, uint32_t address);