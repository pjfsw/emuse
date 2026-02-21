#pragma once

#include <stdint.h>

#define EMU_STAT_WINDOW_SIZE 60

typedef struct {
    uint64_t lastCycleCount;
    uint64_t lastTicks;
    uint32_t lastFrameCount;
    uint32_t renderFrames;
    float currentFps;
    float currentMhz;
    float renderFps;
} EmuStats;

void emuStatsInit(EmuStats *stats, uint64_t ticks);

void emuStatsUpdate(EmuStats *stats, uint64_t totalCyclesRun, uint64_t ticks, uint32_t lastFrameCount);

float emuStatsCurrentMhz(EmuStats *stats);

float emuStatsCurrentFps(EmuStats *stats);

float emuStatsRenderFps(EmuStats *stats);