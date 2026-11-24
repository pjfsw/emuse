#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "booleanfunc.h"
#include "intfunc.h"

typedef struct {
    BooleanFunc isActiveFunc;
    void *isActiveFuncUserdata;
    IntFunc timerSettingFunc;
    void *timerSettingUserdata;
    uint32_t cpuClockSpeed;
    int lastValue;
    bool isInterrupt;
    uint64_t value;
    uint64_t step;
    uint16_t bound;
} Timer;

void timerInit(Timer *timer, uint32_t cpuClockSpeed, BooleanFunc isActiveFunc, void *isActiveFuncUserdata,
    IntFunc timerSettingFunc, void *timerSettingUserdata);

void timerClock(void *userdata, int clocks);

bool timerIsInterrupt(void *userdata);

uint8_t timerReadByte(void *userdata, uint32_t address);

uint16_t timerReadWord(void *userdata, uint32_t address);

void timerWriteByte(void *userdata, uint32_t address, uint8_t byte);

void timerWriteWord(void *userdata, uint32_t address, uint16_t word);

