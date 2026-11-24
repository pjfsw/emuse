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


static const uint32_t vgaPalette[16] = {
    0x000000FF, // 0  Black
    0xB60000FF, // 1  Dark red
    0x00B600FF, // 2  Dark green
    0xB64900FF, // 3  Brown
    0x0000B6FF, // 4  Dark blue
    0xB600B6FF, // 5  Dark magenta
    0x00B6B6FF, // 6  Dark cyan
    0xB6B6B6FF, // 7  Light gray
    0x494949FF, // 8  Dark gray
    0xFF4949FF, // 9  Bright red
    0x49FF49FF, // A  Bright green
    0xFFFF49FF, // B  Yellow
    0x4949FFFF, // C  Bright blue
    0xFF49FFFF, // D  Bright magenta
    0x49FFFFFF, // E  Bright cyan
    0xFFFFFFFF  // F  White
};

/*static const unsigned char packed_font_data[] = {
    #embed "IBM_VGA_8x16.bin"
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
        vga->cram[i] = rand() & 255;
    }
    vga->fontBank = 0;
    vga->sharedState = sharedState;
    vga->activeWriteBuffer = vga->bufferA;
    vga->pollutionTimer = 0;
}

/*static uint32_t rgbmToRgb32(uint32_t rgbm) {
    uint32_t m = rgbm & 1;              // xxxxxxxM
    uint32_t b = (rgbm & 6) | m;        // xxxxxBBx
    uint32_t g = (rgbm>>3) & 7;         // xxGGGxxx
    uint32_t r = ((rgbm>>5) & 6) | m;   // RRxxxxxx
    return ((r*255/7)<<24)|((g*255/7)<<16)|((b*255/7)<<8)|0xff;
}*/

static void renderPixel(Vga *vga) {
    int currentPixel = vga->y * H_VISIBLE + vga->x;    
    int scaledY = (vga->y+(vga->yScroll<<1))&0x1ff;
    int scaledX = vga->x;
    int subPixel = 7 - (vga->x & 7);

    if (subPixel == 4) {
        if (vga->pollutionTimer > 0) {
            vga->charByte = vga->writtenByte;
            vga->attrByte = vga->writtenByte;
        } else {
            int baseOffset = 0x0000;
            int offset = baseOffset + ((scaledX >> 3) + ((scaledY >> 4) << 7));    
            vga->charByte = vga->vram[offset];
            vga->attrByte = vga->cram[offset];
        }            
    } else if (subPixel == 0) {
        vga->color = vga->attrByte;
        if (vga->pollutionTimer > 0) {
            vga->shiftByte = vga->writtenByte;
            vga->pollutionTimer--;
        } else {   
            int baseOffset = vga->fontBank * 4096;                                 
            vga->shiftByte = vga->vram[(vga->charByte<<4)+baseOffset + (scaledY & 15)];
        }
    }
    
    uint8_t colorIdx = (vga->shiftByte & 0x80) >> 7;
    vga->shiftByte <<= 1;
    uint32_t color = vgaPalette[(vga->color >> (colorIdx ? 0 : 4)) & 0xf];
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
        vga->cram[vga->nextAddress] = vga->writtenAttr;
        vga->nextAddress++;
        vga->writtenByte = byte;        
        vga->pollutionTimer = 2;
    } else if (address == VGA_REG_ADDR_HI) {
        vga->nextAddress = (vga->nextAddress & (uint16_t)0x00ff) | ((uint16_t)byte)<<8;
    } else if (address == VGA_REG_ADDR_LO) {
        vga->nextAddress = (vga->nextAddress & (uint16_t)0xff00) | (uint16_t)byte;
    } else if (address == VGA_REG_ATTR) {
        vga->writtenAttr = byte;
    } else if (address == VGA_YSCROLL) {
        vga->yScroll = byte;
    } else if (address = VGA_FONTBANK) {
        vga->fontBank = byte & 0x0f;
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
