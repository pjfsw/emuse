#include <stdbool.h>
#include <SDL3/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include "audio.h"

static float getNextSample(Audio *audio) {
    // 1. Calculate exact CPU cycles needed for THIS sample
    int cyclesForSample = audio->baseCycles;
    audio->audioFractionalAcc += audio->remainder;

    if (audio->audioFractionalAcc >= audio->sampleFreq) {
        cyclesForSample++;
        audio->audioFractionalAcc -= audio->sampleFreq;
    }
    // 2. Adjust for how much the CPU overshot the last time
    int targetCycles = cyclesForSample - audio->cpuCycleDebt;
    int actualCyclesRun = 0;

    // 3. Run CPU instructions until we meet or exceed the target
    while (actualCyclesRun < targetCycles) {
        // StepCPU() executes ONE full instruction and returns how many cycles it took
        int cycles = audio->mainTicker(audio->mainTickerUserdata);

        // 4. Catch up the PPU based on exact CPU cycles run
        audio->videoFractionalAcc += (int64_t)cycles * audio->videoFreq;
        int videoTicks = audio->videoFractionalAcc / audio->cpuFreq;
        audio->videoFractionalAcc %= audio->cpuFreq;

        if (videoTicks > 0) {
            audio->videoTicker(audio->videoTickerUserdata, videoTicks);
        }

        // 5. Catch up the internal audio hardware state
        // TBD StepAudioHardware(cycles), might happen elsewhere so leave it for now

        actualCyclesRun += cycles;
    }

    // 6. Save the instruction overhang for the next loop iteration!
    audio->cpuCycleDebt = actualCyclesRun - targetCycles;

    // 7. We are exactly at the 1/48000th of a second mark. Sample the audio output.
    return audio->sampleSource(audio->sampleSourceUserdata);
}

static void legacyAudioCallback(Audio *audio, float *stream, int len) {
    if (SDL_GetAtomicInt(&audio->sharedState->runRequested) == 0) {
        memset(stream, 0, len * sizeof(float));
        return;
    }

    SDL_SetAtomicInt(&audio->sharedState->audioIsBusy, 1);

    for (int i = 0; i < len; i++) {
        if (SDL_GetAtomicInt(&audio->sharedState->runRequested) == 0) {
            memset(&stream[i], 0, (len - i) * sizeof(float));
            break;
        } 
        stream[i] = getNextSample(audio);
    }
    SDL_SetAtomicInt(&audio->sharedState->audioIsBusy, 0);
}

static void audioCallback(void* userdata, SDL_AudioStream *stream, int additional_amount, int total_amount) {
    Audio *audio = (Audio*)userdata;
    int len = additional_amount/sizeof(float);
    const int bufLen = 2048;
    float buf[bufLen];
    while (len > 0) {
        int thisLen = len > bufLen ? bufLen : len;
        legacyAudioCallback(audio, buf, thisLen);
        SDL_PutAudioStreamData(stream, buf, thisLen*sizeof(float));    
        len -= thisLen;
    }
}

static bool initDevice(Audio *audio) {
    SDL_AudioSpec spec;
    spec.format = SDL_AUDIO_F32;
    spec.channels = 1; // MONO for now
    spec.freq = audio->sampleFreq;

    SDL_SetHint(SDL_HINT_AUDIO_DEVICE_SAMPLE_FRAMES, "256");

    if (!(audio->stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, audioCallback, audio))) {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to open audio stream: %s", SDL_GetError());        
        return false;
    }
    return true;
}

bool audioInit(Audio *audio, int cpuFreq, int sampleFreq, int videoFreq, MainTicker mainTicker,
    void *mainTickerUserdata, VideoTicker videoTicker, void *videoTickerUserdata, SampleSource sampleSource,
    void *sampleSourceUserdata, SharedState *sharedState) {
    memset(audio, 0, sizeof(Audio));
    audio->cpuFreq = cpuFreq;
    audio->sampleFreq = sampleFreq;
    audio->videoFreq = videoFreq;
    audio->mainTicker = mainTicker;
    audio->mainTickerUserdata = mainTickerUserdata;
    audio->videoTicker = videoTicker;
    audio->videoTickerUserdata = videoTickerUserdata;
    audio->sampleSource = sampleSource;
    audio->sampleSourceUserdata = sampleSourceUserdata;
    audio->sharedState = sharedState;
    audio->baseCycles = cpuFreq / sampleFreq; // 416
    audio->remainder = cpuFreq % sampleFreq; // 32000

    if (!initDevice(audio)) {
        return false;
    }
    return true;
}

void audioDestroy(Audio *audio) {
    if (audio->stream != NULL) {
        SDL_DestroyAudioStream(audio->stream);
    }
}