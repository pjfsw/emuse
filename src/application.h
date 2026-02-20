#pragma once

#include <stdbool.h>
#include <SDL3/SDL.h>
#include "font.h"

typedef struct {
    Font font;
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    bool is_fullscreen;
} Application;

bool appInit(Application *app);
void appRun(Application *app);
void appDestroy(Application *app);




