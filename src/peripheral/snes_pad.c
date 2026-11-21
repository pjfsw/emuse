#include <string.h>
#include "snes_pad.h"

void snesPadInit(SnesPad *pad, BooleanFunc latchFunc, BooleanFunc clockFunc, void *funcUserdata) {
    memset(pad, 0, sizeof(SnesPad));
    pad->latchFunc = latchFunc;
    pad->clockFunc = clockFunc;
    pad->funcUserdata = funcUserdata;
    pad->shiftReg = 0xffff;
    SDL_SetAtomicInt(&pad->buttons, 0);
}

static void loadButtons(SnesPad *pad) {
    // Bits 12-15 are unused and always read as released (high)
    uint16_t buttons = (uint16_t)SDL_GetAtomicInt(&pad->buttons) & 0x0fff;
    pad->shiftReg = ~buttons;
}

void snesPadClock(void *userdata, int clocks) {
    SnesPad *pad = (SnesPad*)userdata;

    bool clock = pad->clockFunc(pad->funcUserdata);
    if (pad->latchFunc(pad->funcUserdata)) {
        // Parallel load while latch is high, clock is ignored
        loadButtons(pad);
    } else if (clock && !pad->lastClock) {
        // Rising edge shifts next bit, serial input is tied low
        pad->shiftReg >>= 1;
    }
    pad->lastClock = clock;
}

bool snesPadGetData(SnesPad *pad) {
    return pad->shiftReg & 1;
}

void snesPadSetButtons(SnesPad *pad, uint16_t buttons) {
    SDL_SetAtomicInt(&pad->buttons, buttons);
}
