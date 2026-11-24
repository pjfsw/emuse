#include "timer.h"
#include <stdio.h>

static const uint64_t TIMER_CLOCK_SPEED = 614400ULL;
static const uint64_t FRAC_SIZE = 40;
static const int TIMER_SETTING_75HZ = 1;
static const int TIMER_SETTING_150HZ = 2;
static const int TIMER_SETTING_300Hz = 3;

void timerInit(Timer *timer, uint32_t cpuClockSpeed, BooleanFunc isActiveFunc, void *isActiveFuncUserdata,
    IntFunc timerSettingFunc, void *timerSettingUserdata) {
    timer->cpuClockSpeed = cpuClockSpeed;
    timer->isActiveFunc = isActiveFunc;
    timer->isActiveFuncUserdata = isActiveFuncUserdata;
    timer->timerSettingFunc = timerSettingFunc;
    timer->timerSettingUserdata = timerSettingUserdata;
    timer->step = (TIMER_CLOCK_SPEED << FRAC_SIZE) / cpuClockSpeed;
}

void timerClock(void *userdata, int clocks) {
    Timer *timer = (Timer*)userdata;    
    if (!timer->isActiveFunc(timer->isActiveFuncUserdata)) {
        timer->value = 0;
        return;
    }
    uint64_t clockStep = ((uint64_t)clocks) * timer->step;
    timer->value += clockStep;
    int timerSetting = timer->timerSettingFunc(timer->timerSettingUserdata);
    int timerBit = 0;
    if (timerSetting > 0) {
        timerBit = (timer->value >> (FRAC_SIZE+13-timerSetting)) & 1;
    }
    if ((timerBit == 1 && (timer->lastValue == 0))) {
        timer->isInterrupt = true;
    }
    timer->lastValue = timerBit;   
}

bool timerIsInterrupt(void *userdata) {
    Timer *timer = (Timer*)userdata;
    return timer->isInterrupt;
}

static void acknowledgeIrq(Timer *timer) {
    timer->isInterrupt = false;
}

uint8_t timerReadByte(void *userdata, uint32_t address) {
    acknowledgeIrq((Timer*)userdata);
    return 0;
}

uint16_t timerReadWord(void *userdata, uint32_t address) {
    acknowledgeIrq((Timer*)userdata);
    return 0;
}

void timerWriteByte(void *userdata, uint32_t address, uint8_t byte) {
    acknowledgeIrq((Timer*)userdata);
}

void timerWriteWord(void *userdata, uint32_t address, uint16_t word) {
    acknowledgeIrq((Timer*)userdata);
}

