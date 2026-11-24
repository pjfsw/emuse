#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct PtsHandler PtsHandler; 

PtsHandler *ptsHandlerCreate();

void ptsWriteByte(PtsHandler *pts, uint8_t data);

bool ptsIsByteAvailable(PtsHandler *pts);

bool ptsReadByte(PtsHandler *pts, uint8_t *byte);

void ptsSetRts(PtsHandler *pts, bool rts);

