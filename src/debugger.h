#pragma once

#include "probe.h"
#include "font.h"

typedef struct {
    DisassemblyFunc disassemblyFunc;
    CpuStateFunc cpuStateFunc;
    void *probeUserdata;
    SDL_Renderer *renderer;
    SDL_Texture *backdrop;
    Font *font;
    int width;
    int height;
} Debugger;

void debuggerInit(Debugger *debugger, DisassemblyFunc disassemblyFunc, CpuStateFunc cpuStateFunc, void *probeUserdata,
    SDL_Renderer *renderer, Font *font, int width, int height);

void debuggerUpdate(Debugger *debugger);

int debuggerWidth(Debugger *debugger);

int debuggerHeight(Debugger *debugger);

SDL_Texture *debuggerTexture(Debugger *debugger);