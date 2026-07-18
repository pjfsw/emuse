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