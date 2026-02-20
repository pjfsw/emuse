#pragma once

#include <SDL3/SDL.h>

typedef struct {
    SDL_Texture *font[2048];
    SDL_Renderer *renderer;
} Font;

void fontInit(Font *font, SDL_Renderer *renderer);

void fontDestroy(Font *font);

void fontWrite(Font *font, char *str, int x, int y, uint32_t color);