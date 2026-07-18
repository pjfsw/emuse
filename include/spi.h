#pragma once

#include <stdint.h>
#include "booleanfunc.h"

typedef struct {
    BooleanFunc csFunc;
    BooleanFunc siFunc;
    BooleanFunc clkFunc;
    void *funcUserdata;
    uint8_t miso;
} Spi;

void spiInit(Spi *spi, BooleanFunc csFunc, BooleanFunc siFunc, BooleanFunc clkFunc, void *funcUserdata);

bool spiIsClock(Spi *spi);

bool spiIsMosi(Spi *spi);

// ACTIVE HIGH!! (i.e true <=> CS is asserted <=> 0V )
bool spiIsCs(Spi *spi);