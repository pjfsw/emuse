#include <stdio.h>
#include <stdbool.h>

#include "application.h"
#include "texture.h"

// --- Dummy Callbacks for Testing ---
static int dummyMainTicker(void *userdata) {
    // Fake executing an instruction that takes 4 cycles
    return 4; 
}
static int dummyVideoTicker(void *userdata, int ticks) {
    Application *app = (Application*)userdata;
    
    // Total ticks per frame at 60Hz and 25.175MHz is roughly 419,583. 
    // We only have 320x240 = 76,800 visible pixels. The rest is "V-Blank".
    const int TOTAL_TICKS_PER_FRAME = app->videoFreq / 60; 

    for (int i = 0; i < ticks; i++) {
        // If we are in the visible screen area, draw a pixel
        if (app->currentPixel < (EMU_WIDTH * EMU_HEIGHT)) {
            app->activeWriteBuffer[app->currentPixel] = app->dummyColor;
        }

        app->currentPixel++;

        // Have we hit the end of the full timing frame (including V-Blank)?
        if (app->currentPixel >= TOTAL_TICKS_PER_FRAME) {
            
            // 1. Hand off the completed buffer to the main thread!
            SDL_SetAtomicPointer(&app->sharedState.readyFramePtr, app->activeWriteBuffer);

            // 2. Swap the PPU's target to the other buffer
            app->activeWriteBuffer = (app->activeWriteBuffer == app->bufferA) ? app->bufferB : app->bufferA;
            
            // 3. Reset the pixel counter and shift the dummy color slightly
            app->currentPixel = 0;
            app->dummyColor += 0x00010500; // Shift colors slowly so you can see it animating
        }
    }
    return 0;
}

static float dummySampleSource(void *userdata) {
    // Fake returning an audio sample (silence)
    return 0.0f; 
}

static bool initAudio(Application *app) {
    // Initialize audio system (20MHz CPU, 48kHz audio, 25.175MHz video)
    if (!audioInit(&app->audio, app->cpuFreq, app->sampleFreq, app->videoFreq,
                   dummyMainTicker, app, 
                   dummyVideoTicker, app, 
                   dummySampleSource, app, 
                   &app->sharedState)) {
        SDL_Log("Failed to initialize audio timing engine.");
        return false;
    }
    // Start the audio thread pumping!
    SDL_ResumeAudioStreamDevice(app->audio.stream);    
    return true;
}


static void initVideo(Application *app) {
    app->activeWriteBuffer = app->bufferA;
    app->currentPixel = 0;
    app->dummyColor = 0xFF0000FF;  // Start with solid Red (RGBA)
    SDL_SetAtomicPointer(&app->sharedState.readyFramePtr, NULL);

    // Create a streaming texture matching our emulator's internal resolution
    app->emuTexture =
        SDL_CreateTexture(app->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, EMU_WIDTH, EMU_HEIGHT);
}

bool appInit(Application *app) {
    memset(app, 0, sizeof(Application));

    app->cpuFreq = 20000000;
    app->sampleFreq = 48000;
    app->videoFreq = 25175000;
    
    app->width = 854;
    app->height = 480;
    if (!SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_EVENTS)) {
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

    if (!(app->renderer = SDL_CreateRenderer(app->window, NULL))) {
        return false;
    }

    if (!(app->target = textureCreate(app->renderer, app->width, app->height))) {
        return false;
    }
    SDL_SetTextureScaleMode(app->target, SDL_SCALEMODE_NEAREST);    

    fontInit(&app->font, app->renderer);

    // --- Init Emulator Sync State ---
    SDL_SetAtomicInt(&app->sharedState.runRequested, 1);
    SDL_SetAtomicInt(&app->sharedState.audioIsBusy, 0);
    app->is_stepping = false;    

    if (!initAudio(app)) {
        return false;
    }

    initVideo(app);

    app->running = true;
    app->is_fullscreen = false;
    return true;
}

void appDestroy(Application *app) {
    audioDestroy(&app->audio); // Clean up the audio stream!    
    fontDestroy(&app->font);
    SDL_DestroyTexture(app->target);
    SDL_DestroyRenderer(app->renderer);
    SDL_DestroyWindow(app->window);
    SDL_Quit();
}

static void toggleStepping(Application *app) {
    // --- The Step Handshake ---
    app->is_stepping = !app->is_stepping;

    if (app->is_stepping) {
        // 1. Request pause
        SDL_SetAtomicInt(&app->sharedState.runRequested, 0);

        // 2. Spinlock wait for audio thread to acknowledge and back off
        while (SDL_GetAtomicInt(&app->sharedState.audioIsBusy) == 1) {
            SDL_Delay(1);
        }
        SDL_Log("Emulator PAUSED. Audio thread safely locked out.");
    } else {
        // 1. Reset accumulators so the math doesn't freak out and fast-forward!
        app->audio.audioFractionalAcc = 0;
        app->audio.cpuCycleDebt = 0;
        app->audio.videoFractionalAcc = 0;

        // 2. Resume audio-driven execution
        SDL_SetAtomicInt(&app->sharedState.runRequested, 1);
        SDL_Log("Emulator RESUMED. Audio thread in control.");
    }
}

static void singleStep(Application *app) {
    // Because audioIsBusy == 0, we have 100% exclusive access to the system.
    int cycles = app->audio.mainTicker(app->audio.mainTickerUserdata);

    // Catch up the video manually for this single instruction
    app->audio.videoFractionalAcc += (int64_t)cycles * app->audio.videoFreq;
    int videoTicks = app->audio.videoFractionalAcc / app->audio.cpuFreq;
    app->audio.videoFractionalAcc %= app->audio.cpuFreq;

    if (videoTicks > 0) {
        app->audio.videoTicker(app->audio.videoTickerUserdata, videoTicks);
    }

    SDL_Log("Single-stepped %d CPU cycles. Video ticked %d times.", cycles, videoTicks);
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

            if (key == SDLK_F12) {
                toggleStepping(app);
            }
            // --- Single Step Execution ---
            if ((key == SDLK_F5) || (key == SDLK_F6) && app->is_stepping) {
                singleStep(app);
            }
        }
    }
}

static void renderEmulatorOutput(Application *app) {
    // In application.c -> render()

    // --- 1. Atomically grab the finished frame AND clear the slot to NULL ---
    uint32_t *readyFrame = (uint32_t *)SDL_SetAtomicPointer((void **)&app->sharedState.readyFramePtr, NULL);

    if (readyFrame != NULL) {
        // 2. Upload the pixels to our emulator texture
        SDL_UpdateTexture(app->emuTexture, NULL, readyFrame, EMU_WIDTH * sizeof(uint32_t));
    }

    // --- 3. Draw the emulator texture to the screen ---
    SDL_FRect destRect = {0, 0, EMU_WIDTH, EMU_HEIGHT};
    SDL_RenderTexture(app->renderer, app->emuTexture, NULL, &destRect);
}

static void render(Application* app) {
    SDL_SetRenderTarget(app->renderer, app->target);
    // Clear to Black
    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
    SDL_RenderClear(app->renderer);    

    renderEmulatorOutput(app);

    fontWrite(&app->font, "Hello world", 0, 0, 0xffffffff);
    fontWrite(&app->font, "Maybe red?", 0,8, 0xff0000ff);

    SDL_SetRenderTarget(app->renderer, NULL);
    SDL_RenderTexture(app->renderer, app->target, NULL, NULL);

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
