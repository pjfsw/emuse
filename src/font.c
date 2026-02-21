#include "font.h"
#include "texture.h"

static const unsigned char packed_font_data[] = {
    #embed "font.bin"
};

void fontInit(Font *font, SDL_Renderer *renderer) {
    font->renderer = renderer;
    uint32_t fg = 0xffffffff;  // white
    uint32_t bg = 0x00000000;  // transparent or black
    for (int i = 0 ; i < 256; i++) {
        uint32_t pixels[8 * 8];    // RGBA8888 output        
        for (int y = 0; y < 8; y++) {
            uint8_t row = packed_font_data[i*8+y];
            for (int x = 0; x < 8; x++) {
                int bit = (row >> (7 - x)) & 1;
                pixels[y * 8 + x] = bit ? fg : bg;
            }
        }        
        font->font[i] = textureCreate(renderer, 8, 8);
        SDL_UpdateTexture(font->font[i], NULL, pixels, 8 * sizeof(uint32_t));
    }
}

void fontDestroy(Font *font) {
    for (int i = 0; i < 256; i++) {
        SDL_DestroyTexture(font->font[i]);
    }
}

static void fontPutc(Font *font, uint8_t c, int x, int y, uint32_t color) {
    SDL_Texture *t = font->font[c];
    uint32_t r = (color >> 24) & 0xff;
    uint32_t g = (color >> 16) & 0xff;
    uint32_t b = (color >> 8) & 0xff;
    uint32_t a = color & 0xff;
    SDL_SetTextureColorMod(t, r, g, b);
    SDL_SetTextureAlphaMod(t, a);  // optional
    SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
    SDL_FRect dst = {
        .x = x,
        .y = y,
        .w = 8,
        .h = 8
    };

    SDL_RenderTexture(font->renderer, t, NULL, &dst);
}

void fontWrite(Font *font, char *str, int x, int y, uint32_t color) {
    for (int i = 0 ; i < strlen(str); i++) {
        fontPutc(font, str[i], x + i * 8, y, color);
    } 
}