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
#include "cpu.h"
#include "reset.h"
#include "booleanfunc.h"
#include "snes_pad.h"
#include "terminal.h"

typedef struct {
    Cpu *cpu;
    MainTicker mainTicker;
    void *mainTickerUserdata;
    ResetFunc resetFunc;
    void *resetUserdata;
    int cpuFreq;
    int videoFreq;
    int sampleFreq;
    EmuStats stats;
    Font font;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *target;
    SDL_Texture *emuTexture;
    int width;
    int height;
    Audio audio;
    Vga *vga;
    SharedState sharedState;
    Debugger debugger;
    BooleanFunc ledFunc;
    void *ledFuncUserdata;
    SnesPad *snesPad;
    SDL_Gamepad *gamepad;
    uint16_t snesButtons;
    Terminal terminal;
    const char *uploadFile;     // Sent with Ctrl+Shift+U, NULL if not given
    char statusMessage[81];     // Shown in the terminal banner until statusUntil
    uint64_t statusUntil;
    bool running;
    bool is_fullscreen;
    bool is_stepping; // true = paused, false = running  
    bool showSpeed;   
    bool showMemory;
    bool showHardware;
    bool pixelPerfect;
    bool showTerminal;
} Application;

bool appInit(Application *app, Cpu *cpu, Vga *vga, MainTicker mainTicker, void *mainTickerUserdata, ResetFunc resetFunc,
    void *resetUserdata, int cpuFreq, int videoFreq, int sampleFreq, BooleanFunc ledFunc, void *ledFuncUserdata,
    SnesPad *snesPad);
void appSetTerminalAttached(Application *app, bool attached);
void appSetUploadFile(Application *app, const char *path);
void appResume(Application *app);
void appRun(Application *app);
void appDestroy(Application *app);




