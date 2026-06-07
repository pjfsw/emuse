#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct PtsHandler PtsHandler; 

PtsHandler *ptsHandlerCreate();

void ptsWriteByte(PtsHandler *pts, uint8_t data);

bool ptsIsByteAvailable(PtsHandler *pts);

uint8_t ptsReadByte(PtsHandler *pts);
