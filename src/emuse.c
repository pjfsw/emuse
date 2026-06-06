#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include "application.h"
#include "m68k.h"
#include "bus.h"
#include "memory.h"
#include "addressable_latch.h"

typedef struct {
    uint32_t cpuFreq;
    char romFile[1024];
} Args;

void readArgs(Args *args, int argc, char *argv[]) {
    memset(args, 0, sizeof(Args));
    args->cpuFreq = 10000000;

    int c;
    while ((c = getopt(argc, argv, "z:r:")) != -1) {
        switch(c) {
            case 'z':
                args->cpuFreq = atoi(optarg);
                break;
            case 'r':
                strcpy(args->romFile, optarg);
                break;
        }
    }
}

size_t loadFile(const char *filename, void *buffer, size_t maxSize) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        return 0;
    }

    size_t bytesRead = fread(buffer, 1, maxSize, file);

    fclose(file);

    return bytesRead;
}

static const uint32_t AREG_BASE = 0xd00000;
static const uint32_t OVR_ADDRESS = AREG_BASE + 9;
static const uint16_t OVR_BIT_VALUE = 2;

static bool isRomOverlay(void *userdata) {
    AddrLatch *latch = (AddrLatch*)userdata;
    return !(addrLatchGetValue(latch, OVR_ADDRESS) & OVR_BIT_VALUE);
}

static bool isNotRomOverlay(void *userdata) {
    return !isRomOverlay(userdata);
}

int main(int argc, char* argv[]) {
    Args args;
    readArgs(&args, argc, argv);
    Application app = {0};

    Bus bus;
    busInit(&bus);
    M68k cpu;
    RwFunc rwFunc = {
        .rb = busReadByte,
        .rw = busReadWord,
        .wb = busWriteByte,
        .ww = busWriteWord
    };

    m68kInit(&cpu, rwFunc, &bus);
    busAddCpu(&bus, m68kClock, &cpu);
    busAddResetFunc(&bus, m68kReset, &cpu);

    Memory rom;
    uint32_t romSize = 0x10000;
    memoryInit(&rom, romSize);
    if (args.romFile[0] == 0) {
        fprintf(stderr, "Error no ROM file specified! ( Use -r <rom path> )\n");
        return 1;
    }
    size_t romFileSize = loadFile(args.romFile, rom.data, rom.size);
    if (romFileSize > 0) {
        printf("Loading ROM file \"%s\" (%d bytes)\n", args.romFile, romFileSize);
    } else {
        fprintf(stderr, "ROM file empty or not found: \"%s\"\n", args.romFile);
        return 1;
    }
    ReadWriteMappingKey mappingKey;
    memset(&mappingKey, 0, sizeof(ReadWriteMappingKey));

    AddrLatch outReg;
    addrLatchInit(&outReg, 2);

    mappingKey.start = AREG_BASE;
    mappingKey.end = AREG_BASE + 0x100000;
    mappingKey.userdata = &outReg;
    busAddResetFunc(&bus, addrLatchReset, &outReg);
    busAddWriteFunc(&bus, addrLatchWriteByte, addrLatchWriteWord, mappingKey);

    mappingKey.start = 0xf00000;
    mappingKey.end = 0x1000000;
    mappingKey.userdata = &rom;
    busAddReadFunc(&bus, memoryReadByte, memoryReadWord, mappingKey);

    mappingKey.start = 0x000000;
    mappingKey.end =   0x100000;
    mappingKey.userdata = &rom;
    mappingKey.conditionFunc = isRomOverlay;
    mappingKey.conditionFuncUserdata = &outReg;
    busAddReadFunc(&bus, memoryReadByte, memoryReadWord, mappingKey);
   

    Memory ram;
    uint32_t ramSize = 1048576;
    memoryInit(&ram, ramSize);

    mappingKey.start = 0x000000;
    mappingKey.end =   0x100000;
    mappingKey.userdata = &ram;
    mappingKey.conditionFunc = isNotRomOverlay;
    mappingKey.conditionFuncUserdata = &outReg;
    busAddReadFunc(&bus, memoryReadByte, memoryReadWord, mappingKey);
    busAddWriteFunc(&bus, memoryWriteByte, memoryWriteWord, mappingKey);

    const int sampleFreq = 48000;
    const int videoFreq = 25175000;
    if (!appInit(&app, &cpu.cpu, busClock, &bus, busReset, &bus, args.cpuFreq, videoFreq, sampleFreq)) {
        return 1;
    }
    appRun(&app);
    appDestroy(&app);
    return 0;
}