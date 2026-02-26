#include <string.h>
#include "m68k.h"

static int clock(void *userdata) {
    M68k *cpu = (M68k *)userdata;
    return 4;
}

void m68k_init(M68k *m68k) {
    memset(m68k, 0, sizeof(M68k));
    m68k->cpu.mainTicker = clock;
}