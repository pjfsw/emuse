#include <string.h>
#include "emustats.h"

void emuStatsInit(EmuStats *stats, uint64_t ticks) {
    memset(stats, 0, sizeof(EmuStats));
    stats->lastTicks = ticks;
}

void emuStatsUpdate(EmuStats *stats, uint64_t totalCyclesRun, uint64_t ticks, uint32_t frameCount) {
    uint64_t timeDeltaNS = ticks - stats->lastTicks;
    stats->renderFrames++;

    if (timeDeltaNS > 1000000000) {
        uint32_t framesDelta = frameCount - stats->lastFrameCount;

        uint64_t cyclesDelta = totalCyclesRun - stats->lastCycleCount;
        // (Cycles / Nanoseconds) * 1000 = MHz
        stats->currentMhz = ((float)cyclesDelta / timeDeltaNS) * 1000.0f;

        // (Frames / Nanoseconds) * 1,000,000,000 = FPS
        stats->currentFps = (float)(((float)framesDelta / timeDeltaNS) * 1000000000.0f);

        stats->renderFps = (float)(((float)stats->renderFrames / timeDeltaNS) * 1000000000.0f);
        stats->lastFrameCount = frameCount;
        stats->lastCycleCount = totalCyclesRun;
        stats->lastTicks = ticks;
        stats->renderFrames = 0;
    }
}

inline float emuStatsCurrentMhz(EmuStats *stats) {
    return stats->currentMhz;
}

inline float emuStatsCurrentFps(EmuStats *stats) {
    return stats->currentFps;
}

inline float emuStatsRenderFps(EmuStats *stats) {
    return stats->renderFps;
}