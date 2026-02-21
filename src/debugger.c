#include "debugger.h"

#include <SDL3/SDL.h>

#include "texture.h"

void debuggerInit(Debugger *debugger, DisassemblyFunc disassemblyFunc, CpuStateFunc cpuStateFunc, void *probeUserdata,
    SDL_Renderer *renderer, Font *font, int width, int height) {
    debugger->disassemblyFunc = disassemblyFunc;
    debugger->cpuStateFunc = cpuStateFunc;
    debugger->probeUserdata = probeUserdata;
    debugger->renderer = renderer;
    debugger->font = font;
    debugger->width = width;
    debugger->height = height;
    debugger->backdrop = textureCreate(renderer, width, height);
}

void debuggerUpdate(Debugger *debugger) {
    SDL_SetRenderTarget(debugger->renderer, debugger->backdrop);
    SDL_SetRenderDrawColor(debugger->renderer, 0, 0, 0, 191);
    SDL_RenderClear(debugger->renderer);    
    SDL_SetRenderDrawColor(debugger->renderer, 0,0,0,255);
    SDL_FRect windowFrame = {
        .x = 0,
        .y = 0,
        .w = debugger->width-1,
        .h = debugger->height-1
    };
    SDL_RenderRect(debugger->renderer, &windowFrame);

    const uint32_t valueColor = 0xffffffff;
    const uint32_t labelColor = 0x6f8fffff;

    const int maxStates = 8;
    CpuState cpuState[maxStates];
    for (int i = 0; i < debugger->cpuStateFunc(debugger->probeUserdata, cpuState, maxStates); i++) {
        int y = i * 8;
        fontWrite(debugger->font, cpuState[i].label, 8, y, labelColor);
        fontWrite(debugger->font, cpuState[i].value, 17 * 8, y, valueColor);
    }
    const int maxDis = 8;
    Disassembly disassembly[maxDis];
    for (int i = 0; i < debugger->disassemblyFunc(debugger->probeUserdata, disassembly, maxDis); i++) {
        int y = (maxStates + 1 + i) * 8;
        fontWrite(debugger->font, disassembly[i].address, 8, y, labelColor);
        fontWrite(debugger->font, disassembly[i].instruction, 8 * 12, y, valueColor);
    }
}

SDL_Texture *debuggerTexture(Debugger *debugger) {
    return debugger->backdrop;
}

inline int debuggerWidth(Debugger *debugger) {
    return debugger->width;
}

inline int debuggerHeight(Debugger *debugger) {
    return debugger->height;
}
