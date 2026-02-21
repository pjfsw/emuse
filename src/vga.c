#include <stdlib.h>
#include "vga.h"

#define H_VISIBLE 640
#define H_PITCH 1024
#define H_TOTAL 800
#define V_VISIBLE 480
#define V_TOTAL 525

#include <stdint.h>

// PICO-8 Palette
// Format: 0xRRGGBBAA (Alpha in the LSB)
static const uint32_t pico8_palette[16] = {
    0x000000FF, //  0: Black
    0x1D2B53FF, //  1: Dark Blue
    0x7E2553FF, //  2: Dark Purple
    0x008751FF, //  3: Dark Green
    0xAB5236FF, //  4: Brown
    0x5F574FFF, //  5: Dark Gray
    0xC2C3C7FF, //  6: Light Gray
    0xFFF1E8FF, //  7: White
    0xFF004DFF, //  8: Red
    0xFFA300FF, //  9: Orange
    0xFFEC27FF, // 10: Yellow
    0x00E436FF, // 11: Green
    0x29ADFFFF, // 12: Blue
    0x83769CFF, // 13: Indigo / Lavender
    0xFF77A8FF, // 14: Pink
    0xFFCCAAFF  // 15: Peach    
};

static const unsigned char packed_font_data[] = {
    #embed "font.bin"
};

void vgaInit(Vga *vga, SharedState *sharedState) {
    memset(vga, 0, sizeof(Vga));
    for (int i = 0; i < sizeof(vga->vram); i++) {
        vga->vram[i] = rand() & 65535;
    }
    for (int i = 32; i < 128; i++) {
        int c = (i % 2)+1;
        for (int y = 0; y < 8; y++) {
            uint16_t data = 0;
            for (int x = 0; x < 8; x++) {
                data = data << 2;
                if ((packed_font_data[i*8+y] >> (7-x)) & 1) {
                    data |= c;
                }
            }
            vga->vram[i*8+y] = data;                        
        }
    }
    vga->sharedState = sharedState;
    vga->activeWriteBuffer = vga->bufferA;
    vga->dummyColor = 0xFF0000FF;  // Start with solid Red (RGBA)    
}


static uint32_t pal[4] = {
    0x00000000,
    0xFFFFFFFF,
    0x77777777,
    0xFFFFFFFF
};


static void renderPixel(Vga *vga) {
    uint16_t attr = vga->vram[0x6000 + (vga->y >> 3) * (H_PITCH>>3) + (vga->x >> 3)];
    uint16_t scancode = attr & 0xff;
    pal[0] = pico8_palette[(attr >> 8) & 0xf];
    pal[1] = pico8_palette[(attr >> 12) & 0xf];
    uint16_t fontWord = vga->vram[scancode*8+(vga->y&7)];    
    
    int currentPixel = vga->y * H_VISIBLE + vga->x;    

    uint16_t shift = 2 * (7-(currentPixel&7));    
    uint16_t color = (fontWord >> shift) & 3;
    vga->activeWriteBuffer[currentPixel] = pal[color];
}

static void tick(Vga *vga) {
    if ((vga->x < H_VISIBLE) && (vga->y < V_VISIBLE)) {
        renderPixel(vga);
    }
    vga->x++;
    if (vga->x >= H_TOTAL) {
        vga->x = 0;
        vga->y++;
        if (vga->y >= V_TOTAL) {
            vga->frameCount++;
            vga->y = 0;
            // Frame complete
            SDL_SetAtomicPointer((void **)&vga->sharedState->readyFramePtr, (void *)vga->activeWriteBuffer);

            // Swap PPU to the other buffer
            vga->activeWriteBuffer = (vga->activeWriteBuffer == vga->bufferA) ? vga->bufferB : vga->bufferA;
        }
    }
}

void vgaTicker(void *userdata, int ticks) {
    Vga *vga = (Vga*)userdata;
    for (int i = 0; i < ticks; i++) {
        tick(vga);
    }
}

uint32_t vgaGetFrameCount(Vga *vga) {
    return vga->frameCount;
}

int vgaGetWidth(Vga *vga) {
    return H_VISIBLE;
}

int vgaGetHeight(Vga *vga) {
    return V_VISIBLE;
}
