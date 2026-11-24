#pragma once

#include <SDL3/SDL_atomic.h>

typedef struct {
    SDL_AtomicInt runRequested; 
    SDL_AtomicInt audioIsBusy;     
    void *readyFramePtr;  
} SharedState;
