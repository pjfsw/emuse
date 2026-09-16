#pragma once

#include "sharedstate.h"

#define VGA_WIDTH 640
#define VGA_HEIGHT 480

#define VGA_REG_DATA 0
#define VGA_REG_ADDR_HI 1
#define VGA_REG_ADDR_LO 2
#define VGA_COL_BASE 16

typedef struct {
    // --- Emulator Video State ---
    uint8_t vram[0x20000];    
    uint8_t palette[16];
    uint32_t bufferA[VGA_WIDTH * VGA_HEIGHT];
    uint32_t bufferB[VGA_WIDTH * VGA_HEIGHT];
    uint32_t *activeWriteBuffer;
    int x;
    int y;
    SharedState *sharedState;
    uint32_t frameCount;
    uint16_t nextAddress;    
    uint8_t writtenByte;
    uint8_t shiftByte;
    uint8_t pollutionTimer;
} Vga;

void vgaInit(Vga *vga, SharedState *sharedState);

uint32_t vgaGetFrameCount(Vga *vga);

int vgaGetWidth(Vga *vga);

int vgaGetHeight(Vga *vga);

void vgaTicker(void *userdata, int ticks);

void vgaWriteByte(void *userdata, uint32_t address, uint8_t byte);

void vgaWriteWord(void *userdata, uint32_t address, uint16_t word);