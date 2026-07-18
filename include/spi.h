#pragma once

#include <stdint.h>

typedef struct {
    uint8_t miso;
    uint8_t mosi;
    uint8_t clk;
    uint8_t cs;
} Spi;