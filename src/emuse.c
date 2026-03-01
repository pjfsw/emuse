#include "application.h"
#include "m68k.h"
#include "bus.h"

int main(int argc, char* argv[]) {
    Application app = {0};

    M68k cpu;
    m68kInit(&cpu);

    Bus bus;
    busInit(&bus);
    busAddCpu(&bus, m68kClock, &cpu);

    const int cpuFreq = 20000000;
    const int sampleFreq = 48000;
    const int videoFreq = 25175000;

    if (!appInit(&app, &cpu.cpu, busClock, &bus, cpuFreq, videoFreq, sampleFreq)) {
        return 1;
    }
    appRun(&app);
    appDestroy(&app);
    return 0;
}