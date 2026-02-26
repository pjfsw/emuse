#include "bus.h"

#include <string.h>

void busInit(Bus *bus) {
    memset(bus, 0, sizeof(Bus));
}

void busAddClockFunc(Bus *bus, ClockFunc clockFunc, void *userdata) {
    bus->clockFunc[bus->clockFuncCount] = clockFunc;
    bus->clockFuncUserdata[bus->clockFuncCount] = userdata;
    bus->clockFuncCount++;
}

void busAddResetFunc(Bus *bus, ResetFunc resetFunc, void *userdata) {
    bus->resetFunc[bus->resetFuncCount] = resetFunc;
    bus->resetFuncUserdata[bus->resetFuncCount] = userdata;
    bus->resetFuncCount++;
}

void busReset(Bus *bus) {
    for (int i = 0; i < bus->resetFuncCount; i++) {
        bus->resetFunc[i](bus->resetFuncUserdata[i]);
    }
}

void busClock(Bus *bus, int clocks) {
    for (int i = 0; i < bus->clockFuncCount; i++) {
        bus->clockFunc[i]( bus->clockFuncUserdata[i], clocks);
    }
}