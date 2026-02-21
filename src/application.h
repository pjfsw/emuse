#pragma once

#include <SDL3/SDL.h>
#include <stdbool.h>
#include "font.h"
#include "audio.h"
#include "sharedstate.h"
#include "vga.h"
#include "emustats.h"
#include "probe.h"
#include "debugger.h"

typedef struct {
    EmuStats stats;
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
    Vga vga;
    SharedState sharedState;
    Debugger debugger;
    bool is_stepping; // true = paused, false = running  
    bool showSpeed;   
} Application;

bool appInit(Application *app);
void appRun(Application *app);
void appDestroy(Application *app);




