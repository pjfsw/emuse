#include "texture.h"

SDL_Texture *textureCreate(SDL_Renderer *renderer, int w, int h) {
    return SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        w, h
    );
}
