#pragma once

#include "ticker.h"

typedef void (*ResetFunc)(void *user_data);

typedef void (*ClockFunc)(void *user_data, int clocks);

#define MAX_RESET_FUNC 16
#define MAX_CLOCK_FUNC 16

typedef struct {
    MainTicker cpuTicker;
    void *cpuTickerUserdata;
    ResetFunc resetFunc[MAX_RESET_FUNC];
    void *resetFuncUserdata[MAX_RESET_FUNC];
    int resetFuncCount;
    ClockFunc clockFunc[MAX_CLOCK_FUNC];
    void *clockFuncUserdata[MAX_CLOCK_FUNC];
    int clockFuncCount;
} Bus;

void busInit(Bus *bus);

void busAddCpu(Bus *bus, MainTicker cpuTicker, void *cpuTickerUserdata);

void busAddClockFunc(Bus *bus, ClockFunc clockFunc, void *userdata);

void busAddResetFunc(Bus *bus, ResetFunc resetFunc, void *userdata);

void busReset(Bus *bus);

int busClock(void *userdata);