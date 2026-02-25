#include <stdio.h>
#include <stdbool.h>

#include "application.h"
#include "texture.h"

// --- Dummy Callbacks for Testing ---
static int dummyMainTicker(void *userdata) {
    // Fake executing an instruction that takes 4 cycles
    return 4; 
}

static float dummySampleSource(void *userdata) {
    // Fake returning an audio sample (silence)
    return 0.0f; 
}

static bool initAudio(Application *app) {
    // Initialize audio system (20MHz CPU, 48kHz audio, 25.175MHz video)
    if (!audioInit(&app->audio, app->cpuFreq, app->sampleFreq, app->videoFreq,
                   dummyMainTicker, app, 
                   vgaTicker, &app->vga, 
                   dummySampleSource, app, 
                   &app->sharedState)) {
        SDL_Log("Failed to initialize audio timing engine.");
        return false;
    }
    // Start the audio thread pumping!
    SDL_ResumeAudioStreamDevice(app->audio.stream);    
    return true;
}

static int dummyDisassemblyFunc(void *userdata, Disassembly *disassembly, int maxLines) {
    if (maxLines > 0) {
        strncpy(disassembly[0].instruction, "moveq #0,d0", DIS_INSTR_LEN);
        strncpy(disassembly[0].address,  "$0010 0000", DIS_ADDR_LEN);
        disassembly[1].current = false;
        return 1;
    }
    return 0;
}

static int dummyCpuStateFunc(void *userdata, CpuState *cpuState, int maxLines) {
    if (maxLines > 0) {
        strcpy(cpuState[0].label, "D0");
        strcpy(cpuState[0].value, "$12345678");
        return 1;
    }
    return 0;
}

static void initVideo(Application *app) {
    vgaInit(&app->vga, &app->sharedState);
    SDL_SetAtomicPointer(&app->sharedState.readyFramePtr, NULL);

    // Create a streaming texture matching our emulator's internal resolution
    app->emuTexture = SDL_CreateTexture(app->renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        vgaGetWidth(&app->vga),
        vgaGetHeight(&app->vga));
}

bool appInit(Application *app) {
    memset(app, 0, sizeof(Application));

    app->cpuFreq = 20000000;
    app->sampleFreq = 48000;
    app->videoFreq = 25175000;
    
    app->width = 640;
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

    initVideo(app);

    if (!initAudio(app)) {
        return false;
    }

    emuStatsInit(&app->stats, SDL_GetTicksNS());

    debuggerInit(&app->debugger, dummyDisassemblyFunc, dummyCpuStateFunc, app, app->renderer, &app->font, 
        vgaGetWidth(&app->vga)/2, vgaGetHeight(&app->vga));

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

            if (key == SDLK_F8) {
                toggleStepping(app);
            }
            if (key == SDLK_F1) {
                app->showSpeed = !app->showSpeed;
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
        SDL_UpdateTexture(app->emuTexture, NULL, readyFrame, vgaGetWidth(&app->vga) * sizeof(uint32_t));
    } else if (app->is_stepping) {
        // Because the audio thread is locked out, it is 100% safe to read 
        // the active PPU memory mid-draw!
        SDL_UpdateTexture(app->emuTexture, NULL, app->vga.activeWriteBuffer, vgaGetWidth(&app->vga) * sizeof(uint32_t));
    }

    // --- 3. Draw the emulator texture to the screen ---
    SDL_FRect destRect = {0, 0, vgaGetWidth(&app->vga), vgaGetHeight(&app->vga)};
    SDL_RenderTexture(app->renderer, app->emuTexture, NULL, &destRect);
}

static void renderTargetTexture(Application *app) {
    SDL_SetRenderTarget(app->renderer, NULL);
    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
    SDL_RenderClear(app->renderer);

    // 3. Get the current size of the window/renderer
    int winW, winH;
    SDL_GetRenderOutputSize(app->renderer, &winW, &winH);

    // 4. Calculate the scaling factor
    float scaleX = (float)winW / app->width;
    float scaleY = (float)winH / app->height;

    // Use the smaller scale to ensure the image fits entirely inside the window
    float scale = (scaleX < scaleY) ? scaleX : scaleY;

    // 5. Calculate the final dimensions and center position
    SDL_FRect destRect;
    destRect.w = app->width * scale;
    destRect.h = app->height * scale;
    destRect.x = (winW - destRect.w) / 2.0f;  // Center horizontally
    destRect.y = (winH - destRect.h) / 2.0f;  // Center vertically

    // 6. Draw the target texture into the calculated bounding box
    SDL_RenderTexture(app->renderer, app->target, NULL, &destRect);
}

static void render(Application* app) {
    bool showDebug = app->is_stepping;
    if (showDebug) {
        debuggerUpdate(&app->debugger);
    }

    SDL_SetRenderTarget(app->renderer, app->target);    
    // Clear to Black
    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
    SDL_RenderClear(app->renderer);    

    renderEmulatorOutput(app);
    if (showDebug) {
        SDL_FRect destRect = {0, 0, debuggerWidth(&app->debugger), debuggerHeight(&app->debugger)};
        SDL_RenderTexture(app->renderer, debuggerTexture(&app->debugger), NULL, &destRect);
    }

    if (!app->is_stepping && app->showSpeed) {
        int y = vgaGetHeight(&app->vga)-10;
        emuStatsUpdate(&app->stats, app->audio.totalCyclesRun, SDL_GetTicksNS(), vgaGetFrameCount(&app->vga));
        char buf[80];
        SDL_SetRenderDrawBlendMode(app->renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(app->renderer, 0,0,0,160);
        SDL_FRect rect = {
            .x = 0,
            .y = y,
            .w = vgaGetWidth(&app->vga),
            .h = 10
        };
        SDL_RenderFillRect(app->renderer, &rect);
        snprintf(buf,
            sizeof(buf),
            "[F8] Debugger | [F11] Fullscreen | FPS: %2.1f (%7.1f) | CPU: %.2f MHz",
            emuStatsCurrentFps(&app->stats),
            emuStatsRenderFps(&app->stats),
            emuStatsCurrentMhz(&app->stats));
        fontWrite(&app->font, buf, 2, y+1, 0xFFFFFFDF); 
    }
    renderTargetTexture(app);

    // Swap buffers
    SDL_RenderPresent(app->renderer);

    SDL_Delay(1);
}

void appRun(Application *app) {
    while (app->running) {
        handleEvents(app);
        
        // This is where you will eventually call your 68k execution step
        
        render(app);
    }
}
