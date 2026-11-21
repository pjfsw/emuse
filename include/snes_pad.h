#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <SDL3/SDL_atomic.h>
#include "booleanfunc.h"

// Button bits, in the order they are shifted out of the controller
#define SNES_BUTTON_B      (1<<0)
#define SNES_BUTTON_Y      (1<<1)
#define SNES_BUTTON_SELECT (1<<2)
#define SNES_BUTTON_START  (1<<3)
#define SNES_BUTTON_UP     (1<<4)
#define SNES_BUTTON_DOWN   (1<<5)
#define SNES_BUTTON_LEFT   (1<<6)
#define SNES_BUTTON_RIGHT  (1<<7)
#define SNES_BUTTON_A      (1<<8)
#define SNES_BUTTON_X      (1<<9)
#define SNES_BUTTON_L      (1<<10)
#define SNES_BUTTON_R      (1<<11)

typedef struct {
    BooleanFunc latchFunc;
    BooleanFunc clockFunc;
    void *funcUserdata;
    SDL_AtomicInt buttons; // Written by host thread, bit set <=> pressed
    uint16_t shiftReg;     // Active low, bit 0 is on the data line
    bool lastClock;
} SnesPad;

void snesPadInit(SnesPad *pad, BooleanFunc latchFunc, BooleanFunc clockFunc, void *funcUserdata);

void snesPadClock(void *userdata, int clocks);

bool snesPadGetData(SnesPad *pad);

void snesPadSetButtons(SnesPad *pad, uint16_t buttons);
