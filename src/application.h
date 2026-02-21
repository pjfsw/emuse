#pragma once

#include <SDL3/SDL.h>
#include <stdbool.h>
#include "font.h"
#include "audio.h"

#define EMU_WIDTH 640
#define EMU_HEIGHT 480
typedef struct {
    Font font;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *target;
    SDL_Texture *emuTexture;
    bool running;
    bool is_fullscreen;
    int width;
    int height;
    int videoFreq;
    int cpuFreq;
    int sampleFreq;
    Audio audio;
    SharedState sharedState;
    bool is_stepping; // true = paused, false = running    

    // --- Emulator Video State ---
    uint32_t bufferA[EMU_WIDTH * EMU_HEIGHT];
    uint32_t bufferB[EMU_WIDTH * EMU_HEIGHT];
    uint32_t *activeWriteBuffer;
    int currentPixel;
    uint32_t dummyColor;    
} Application;

bool appInit(Application *app);
void appRun(Application *app);
void appDestroy(Application *app);




