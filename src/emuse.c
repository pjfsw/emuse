#include "application.h"
#include "m68k.h"

int main(int argc, char* argv[]) {
    Application app = {0};

    M68k cpu;
    m68k_init(&cpu);
    if (!appInit(&app, &cpu.cpu)) {
        return 1;
    }
    appRun(&app);
    appDestroy(&app);
    return 0;
}