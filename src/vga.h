#pragma once

#include "sharedstate.h"

#define VGA_WIDTH 640
#define VGA_HEIGHT 480

typedef struct {
    // --- Emulator Video State ---
    uint32_t bufferA[VGA_WIDTH * VGA_HEIGHT];
    uint32_t bufferB[VGA_WIDTH * VGA_HEIGHT];
    uint32_t *activeWriteBuffer;
    uint32_t dummyColor;    
    int x;
    int y;
    SharedState *sharedState;
} Vga;

void vgaInit(Vga *vga, SharedState *sharedState);

int vgaGetWidth(Vga *vga);

int vgaGetHeight(Vga *vga);

void vgaTicker(void *userdata, int ticks);