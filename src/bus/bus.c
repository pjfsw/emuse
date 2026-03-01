#include "bus.h"

#include <string.h>

void busInit(Bus *bus) {
    memset(bus, 0, sizeof(Bus));
}

void busAddCpu(Bus *bus, MainTicker cpuTicker, void *cpuTickerUserdata) {
    bus->cpuTicker = cpuTicker;
    bus->cpuTickerUserdata = cpuTickerUserdata;
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

int busClock(void *userdata) {
    Bus *bus = (Bus*)userdata;
    int cycles = bus->cpuTicker(bus->cpuTickerUserdata);
    for (int i = 0; i < bus->clockFuncCount; i++) {
        bus->clockFunc[i]( bus->clockFuncUserdata[i], cycles);
    }
    return cycles;
}