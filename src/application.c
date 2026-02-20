#include <stdio.h>
#include <stdbool.h>
#include "application.h"


bool appInit(Application *app) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return false;
    }

    // Get desktop display mode to calculate 3/4 size
    const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay());
    int w = (mode->w * 3) / 4;
    int h = (mode->h * 3) / 4;

    Uint32 window_flags = SDL_WINDOW_RESIZABLE;

    if (!(app->window = SDL_CreateWindow("emuse", w, h, window_flags))) {
        return false;
    }

    app->renderer = SDL_CreateRenderer(app->window, NULL);
    if (!app->renderer) return false;

    app->running = true;
    app->is_fullscreen = false;

    fontInit(&app->font, app->renderer);
    return true;
}

void appDestroy(Application *app) {
    fontDestroy(&app->font);
    SDL_DestroyRenderer(app->renderer);
    SDL_DestroyWindow(app->window);
    SDL_Quit();
}

static void handleEvents(Application* app) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            app->running = false;
        } 
        else if (event.type == SDL_EVENT_KEY_DOWN) {
            SDL_Keycode key = event.key.key;
            SDL_Keymod mod = event.key.mod;

            // Alt + Enter or F11: Toggle Fullscreen
            if (((mod & SDL_KMOD_ALT) && key == SDLK_RETURN) || key == SDLK_F11) {
                app->is_fullscreen = !app->is_fullscreen;
                SDL_SetWindowFullscreen(app->window, app->is_fullscreen);
            }

            // Alt + Q: Quit
            if ((mod & SDL_KMOD_ALT) && key == SDLK_Q) {
                app->running = false;
            }
        }
    }
}

static void render(Application* app) {
    // Clear to Black
    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
    SDL_RenderClear(app->renderer);

    fontWrite(&app->font, "Hello world", 0, 0, 0xffffffff);

    // Swap buffers
    SDL_RenderPresent(app->renderer);
}

void appRun(Application *app) {
    while (app->running) {
        handleEvents(app);
        
        // This is where you will eventually call your 68k execution step
        
        render(app);
    }
}
