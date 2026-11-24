#include <string.h>
#include "spi.h"

void spiInit(Spi *spi,BooleanFunc csFunc, BooleanFunc siFunc, BooleanFunc clkFunc, void *funcUserdata) {
    memset(spi, 0, sizeof(Spi));    
    spi->csFunc = csFunc;
    spi->siFunc = siFunc;
    spi->clkFunc = clkFunc;
    spi->funcUserdata = funcUserdata;
}

bool spiIsClock(Spi *spi) {
    return spi->clkFunc(spi->funcUserdata);
}

bool spiIsMosi(Spi *spi) {
    return spi->siFunc(spi->funcUserdata);
}

bool spiIsCs(Spi *spi) {
    return spi->csFunc(spi->funcUserdata);
}