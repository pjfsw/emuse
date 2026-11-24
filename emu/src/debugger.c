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
static const uint32_t unknownColor = 0x669CD680;
static const uint32_t activeLineColor =  0xD4D4D433;

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

static const int rowHeight = 16;

static void renderRegisters(Debugger *debugger, int yOffset) {
    const int maxStates = 200;
    const int xofs = 1;
    CpuState gpRegisters[maxStates];
    CpuState statusRegister;
    for (int i = 0; i < debugger->cpuStateFunc(debugger->probeUserdata, gpRegisters, maxStates, &statusRegister); i++) {
        int y = (yOffset + (i % 8)) * rowHeight;
        int x = xofs + (i / 8) * 14;
        renderRegister(debugger, &gpRegisters[i], x*8, y);
    }
    renderRegister(debugger, &statusRegister, xofs*8, (yOffset+9)*rowHeight);
}

static void renderDisassembly(Debugger *debugger, int yOffset, int maxDis) {
    int disassemblyOffset = yOffset;

    Disassembly disassembly[maxDis];
    int currentLine = 0;  
    for (int i = 0; i < debugger->disassemblyFunc(debugger->probeUserdata, disassembly, maxDis, &currentLine); i++) {
        int y = (disassemblyOffset + i) * rowHeight;
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
                    break;
                case SYM_UNKNOWN:
                    color = unknownColor;
                    break;
                default:
                    break;
            }
            fontWrite(debugger->font, part->part, 24 + x * 8, y, color);
            x += strlen(part->part);
        }        
    }
    SDL_SetRenderDrawColor(debugger->renderer,
        (uint8_t)(activeLineColor >> 24),
        (uint8_t)(activeLineColor >> 16),
        (uint8_t)(activeLineColor >> 8),
        (uint8_t)activeLineColor);
    SDL_FRect activeLineRect = {
        .x = 0,
        .y = (disassemblyOffset+currentLine) * 16,
        .w = 320,
        .h = 16 
    };
    SDL_SetRenderDrawBlendMode(debugger->renderer, SDL_BLENDMODE_BLEND);    
    SDL_RenderFillRect(debugger->renderer, &activeLineRect);
}

static void renderMemory(Debugger *debugger, int yOffset) {
    int y = yOffset * rowHeight;
    fontWrite(debugger->font, "MEMORY", 0, y, addressColor);
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
    renderRegisters(debugger, 0);
    const int disassemblyOffset = 11;
    int maxDis = (debuggerHeight(debugger)/16)-disassemblyOffset-1;
    if (debugger->showMemory) {
        maxDis = maxDis - 10;
    }
    renderDisassembly(debugger, disassemblyOffset, maxDis);

    if (debugger->showMemory) {
        renderMemory(debugger, disassemblyOffset + maxDis + 1);        
    }
}

void debuggerSetShowMemory(Debugger *debugger, bool showMemory) {
    debugger->showMemory = showMemory;
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
