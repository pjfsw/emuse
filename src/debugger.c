#include "debugger.h"

#include <SDL3/SDL.h>

#include "texture.h"

static const uint32_t constantColor = 0xB5CEA8FF;
static const uint32_t mnemonicColor = 0x569CD6FF;
static const uint32_t symbolColor = 0xD4D4D4FF;
static const uint32_t addressColor = symbolColor;
static const uint32_t registerColor = mnemonicColor;
static const uint32_t stateValueColor = 0xCCCCCCFF;
static const uint32_t stateLabelColor = mnemonicColor;

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

static void renderRegister(Debugger *debugger, CpuState *reg, int x, int y) {
    fontWrite(debugger->font, reg->label, x, y, stateLabelColor);
    fontWrite(debugger->font, reg->value, x+24, y, stateValueColor);
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


    const int rowHeight = 16;
    const int maxStates = 200;
    const int xofs = 1;
    CpuState gpRegisters[maxStates];
    CpuState statusRegister;
    for (int i = 0; i < debugger->cpuStateFunc(debugger->probeUserdata, gpRegisters, maxStates, &statusRegister); i++) {
        int y = (i % 8) * rowHeight;
        int x = xofs + (i / 8) * 14;
        renderRegister(debugger, &gpRegisters[i], x*8, y);
    }
    renderRegister(debugger, &statusRegister, xofs*8, 9*rowHeight);

    const int maxDis = 16;
    Disassembly disassembly[maxDis];
    for (int i = 0; i < debugger->disassemblyFunc(debugger->probeUserdata, disassembly, maxDis); i++) {
        int y = (10 + 1 + i) * rowHeight;
        fontWrite(debugger->font, disassembly[i].address, 8, y, addressColor);
        int x = strlen(disassembly[i].address);
        for (int j = 0; j < disassembly[i].instruction.count; j++) {
            InstructionPart *part = &disassembly[i].instruction.parts[j];
            uint32_t color = symbolColor;
            switch (part->symbol) {
                case SYM_CONSTANT:
                    color = constantColor;
                    break;
                case SYM_MNEMONIC:
                    color = mnemonicColor;
                    break;
                case SYM_REGISTER:
                    color = registerColor;
                default:
                    break;
            }
            fontWrite(debugger->font, part->part, 16 + x * 8, y, color);
            x += strlen(part->part);
        }
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
