#pragma once

#include "sharedstate.h"

#define VGA_WIDTH 640
#define VGA_HEIGHT 480

typedef struct {
    // --- Emulator Video State ---
    uint16_t vram[32768];    
    uint32_t bufferA[VGA_WIDTH * VGA_HEIGHT];
    uint32_t bufferB[VGA_WIDTH * VGA_HEIGHT];
    uint32_t *activeWriteBuffer;
    uint32_t dummyColor;    
    int x;
    int y;
    SharedState *sharedState;
    uint32_t frameCount;
} Vga;

void vgaInit(Vga *vga, SharedState *sharedState);

uint32_t vgaGetFrameCount(Vga *vga);

int vgaGetWidth(Vga *vga);

int vgaGetHeight(Vga *vga);

void vgaTicker(void *userdata, int ticks);