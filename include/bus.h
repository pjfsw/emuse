#pragma once

typedef void (*ResetFunc)(void *user_data);

typedef void (*ClockFunc)(void *user_data, int clocks);

#define MAX_RESET_FUNC 16
#define MAX_CLOCK_FUNC 16

typedef struct {
    ResetFunc resetFunc[MAX_RESET_FUNC];
    void *resetFuncUserdata[MAX_RESET_FUNC];
    int resetFuncCount;
    ClockFunc clockFunc[MAX_CLOCK_FUNC];
    void *clockFuncUserdata[MAX_CLOCK_FUNC];
    int clockFuncCount;
} Bus;

void busInit(Bus *bus);

void busAddClockFunc(Bus *bus, ClockFunc clockFunc, void *userdata);

void busAddResetFunc(Bus *bus, ResetFunc resetFunc, void *userdata);

void busReset(Bus *bus);

void busClock(Bus *bus, int clocks);