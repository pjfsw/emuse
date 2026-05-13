#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include "application.h"
#include "m68k.h"
#include "bus.h"
#include "memory.h"

typedef struct {
    uint32_t cpuFreq;
    char romFile[1024];
} Args;

void readArgs(Args *args, int argc, char *argv[]) {
    memset(args, 0, sizeof(Args));
    args->cpuFreq = 12000000;

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

int main(int argc, char* argv[]) {
    Args args;
    readArgs(&args, argc, argv);
    Application app = {0};

    M68k cpu;
    m68kInit(&cpu);

    Bus bus;
    busInit(&bus);
    busAddCpu(&bus, m68kClock, &cpu);

    Memory rom;
    uint32_t romSize = 65536;
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
    
    mappingKey.start = 0;
    mappingKey.end = 0x200000;
    mappingKey.userdata = &rom;
    busAddReadFunc(&bus, memoryReadByte, memoryReadWord, mappingKey);
    
    mappingKey.start = 0xe00000;
    mappingKey.end = 0x1000000;
    mappingKey.userdata = &rom;
    busAddReadFunc(&bus, memoryReadByte, memoryReadWord, mappingKey);

    const int sampleFreq = 48000;
    const int videoFreq = 25175000;
    if (!appInit(&app, &cpu.cpu, busClock, &bus, args.cpuFreq, videoFreq, sampleFreq)) {
        return 1;
    }
    appRun(&app);
    appDestroy(&app);
    return 0;
}