#include <stdlib.h>
#include "vga.h"

#define H_VISIBLE 640
#define H_TOTAL 800
#define V_VISIBLE 480
#define V_TOTAL 525

void vgaInit(Vga *vga, SharedState *sharedState) {
    memset(vga, 0, sizeof(Vga));
    vga->sharedState = sharedState;
    vga->activeWriteBuffer = vga->bufferA;
    vga->dummyColor = 0xFF0000FF;  // Start with solid Red (RGBA)    
}

static void renderPixel(Vga *vga) {
    int currentPixel = vga->y * H_VISIBLE + vga->x;    
    vga->activeWriteBuffer[currentPixel] = vga->dummyColor;    
    vga->dummyColor++;
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

int vgaGetWidth(Vga *vga) {
    return H_VISIBLE;
}

int vgaGetHeight(Vga *vga) {
    return V_VISIBLE;
}
