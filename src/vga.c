#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include "vga.h"
#include <stdio.h>

#define H_VISIBLE 640
#define H_PITCH 1024
#define H_TOTAL 800
#define V_VISIBLE 480
#define V_TOTAL 525
#define VGA_HDIV 1
#define VGA_VDIV 1


/*static const unsigned char packed_font_data[] = {
    #embed "font.bin"
};*/

/*uint16_t expand1bpp(uint8_t data, uint8_t color0, uint8_t color1)
{
    uint16_t result = 0;

    color0 &= 3;
    color1 &= 3;

    for (int i = 0; i < 8; i++) {
        result <<= 2;

        if (data & 0x80)
            result |= color1;
        else
            result |= color0;

        data <<= 1;
    }

    return result;
}
*/
void vgaInit(Vga *vga, SharedState *sharedState) {
    memset(vga, 0, sizeof(Vga));
    srand((unsigned)time(NULL));
    for (int i = 0; i < sizeof(vga->vram); i++) {
        vga->vram[i] = rand() & 255;
    }
    for (int i = 0; i < sizeof(vga->palette); i++) {
        vga->palette[i] = rand() & 255;
    }
/*    for (int y = 0; y < 8; y++) {
        uint16_t w = expand1bpp(packed_font_data[8*65+y], 0,2);
        vga->vram[y*256] = w>>8;
        vga->vram[y*256+1] = w&0xff;
    }*/
    vga->sharedState = sharedState;
    vga->activeWriteBuffer = vga->bufferA;
    vga->pollutionTimer = 0;
}

static uint32_t rgbmToRgb32(uint32_t rgbm) {
    uint32_t m = rgbm & 1;              // xxxxxxxM
    uint32_t b = (rgbm & 6) | m;        // xxxxxBBx
    uint32_t g = (rgbm>>3) & 7;         // xxGGGxxx
    uint32_t r = ((rgbm>>5) & 6) | m;   // RRxxxxxx
    return ((r*255/7)<<24)|((g*255/7)<<16)|((b*255/7)<<8)|0xff;
}

static const int blockHeightShift=3;
static const int blockHeight=1<<blockHeightShift;
static const int pixelsPerByteShift = 2; // 1bpp = 3, 2bpp = 2, 4bpp = 1, 8bpp = 0

static void renderPixel(Vga *vga) {
    int currentPixel = vga->y * H_VISIBLE + vga->x;    
    int scaledY = vga->y >> 1;
    int scaledX = vga->x >> 1;
    int subPixel = 7 - (vga->x & 7);

    if (subPixel == 7) {
        if (vga->pollutionTimer > 0) {
            vga->shiftByte = vga->writtenByte;
            vga->pollutionTimer--;
        } else {
            
            int byteX = scaledX >> pixelsPerByteShift;

            vga->shiftByte = vga->vram[(scaledY & (blockHeight - 1)) +
                                       ((scaledY >> blockHeightShift) << (9 - pixelsPerByteShift + blockHeightShift)) +
                                       (byteX << blockHeightShift)];
            //vga->shiftByte =
                //vga->vram[(scaledY & (blockHeight - 1)) + ((scaledY >> blockHeightShift) << (6 + blockHeightShift)) +
                          //((scaledX >> 3) << blockHeightShift)];
        }
    }
    uint8_t colorIdx = (vga->shiftByte & 0xc0) >> 6;
    if ((subPixel & 1) == 0) {
        vga->shiftByte <<= 2;
    }
    uint32_t rgbm = (uint32_t)vga->palette[colorIdx & 3];
    uint32_t color = rgbmToRgb32(rgbm);
    vga->activeWriteBuffer[currentPixel] = color;
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

void vgaWriteByte(void *userdata, uint32_t address, uint8_t byte) {
    Vga *vga = (Vga*)userdata;

    address = (address >> 1) & 0x1f;
    if (address == VGA_REG_DATA) {
        vga->vram[vga->nextAddress] = byte;
        vga->nextAddress++;
        vga->writtenByte = byte;
        vga->pollutionTimer = 2;
    } else if (address == VGA_REG_ADDR_HI) {
        vga->nextAddress = (vga->nextAddress & (uint16_t)0x00ff) | ((uint16_t)byte)<<8;
    } else if (address == VGA_REG_ADDR_LO) {
        vga->nextAddress = (vga->nextAddress & (uint16_t)0xff00) | (uint16_t)byte;
    } else if (address >= VGA_COL_BASE) {
        vga->palette[address & 15] = byte;
    }
}

void vgaWriteWord(void *userdata, uint32_t address, uint16_t word) {
    vgaWriteByte(userdata, (address&0xFFFFFFFE)+1, word & 0xff);
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
