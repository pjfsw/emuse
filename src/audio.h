#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_atomic.h>
#include "ticker.h"
#include "samplesource.h"
#include "sharedstate.h"

typedef struct {
    int cpuFreq;
    int sampleFreq;
    int videoFreq;
    int baseCycles;
    int remainder;
    int audioFractionalAcc;
    int cpuCycleDebt;
    int64_t videoFractionalAcc;
    SDL_AudioStream *stream;
    MainTicker mainTicker;
    void *mainTickerUserdata;
    VideoTicker videoTicker;
    void *videoTickerUserdata;
    SampleSource sampleSource;
    void *sampleSourceUserdata;
    SharedState *sharedState;    
    uint64_t totalCyclesRun;
} Audio;

bool audioInit(Audio *audio, int cpuFreq, int sampleFreq, int videoFreq, MainTicker mainTicker,
    void *mainTickerUserdata, VideoTicker videoTicker, void *videoTickerUserdata, SampleSource sampleSource,
    void *sampleSourceUserdata, SharedState *sharedState);

void audioDestroy(Audio *audio);