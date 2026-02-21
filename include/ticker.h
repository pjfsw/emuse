#pragma once

// Tick the main system one instruction and return the number of cycles that was executed
typedef int (*MainTicker)(void *userdata);

// Tick the video a number of pixels
typedef int (*VideoTicker)(void *userdata, int ticks);
