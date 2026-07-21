#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "application.h"
#include "m68k.h"
#include "bus.h"
#include "memory.h"
#include "addressable_latch.h"
#include "uart.h"
#include "mmc.h"
#include "input_reg.h"
#include "sector_storage.h"
#include "file_inject.h"

typedef struct {
    uint32_t cpuFreq;
    char romFile[1024];
    char mmcFile[1024];
} Args;

void readArgs(Args *args, int argc, char *argv[]) {
    memset(args, 0, sizeof(Args));
    args->cpuFreq = 12000000;

    int c;
    while ((c = getopt(argc, argv, "z:r:m:")) != -1) {
        switch(c) {
            case 'z':
                args->cpuFreq = atoi(optarg);
                break;
            case 'r':
                strcpy(args->romFile, optarg);
                break;
            case 'm':
                strcpy(args->mmcFile, optarg);
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

static const uint32_t UART_BASE = 0xb00000;
static const uint32_t AREG_BASE = 0xd00000;
static const uint32_t OVR_ADDRESS = AREG_BASE + 9;
static const uint32_t SPI_CS_ADDRESS = AREG_BASE + 13;
static const uint32_t SPI_MOSI_CLK_ADDRESS = AREG_BASE + 15;

static const uint16_t OVR_BIT_VALUE = 2;

static bool isRomOverlay(void *userdata) {
    AddrLatch *latch = (AddrLatch*)userdata;
    return !(addrLatchGetValue(latch, OVR_ADDRESS) & OVR_BIT_VALUE);
}

static bool isNotRomOverlay(void *userdata) {
    return !isRomOverlay(userdata);
}

static bool isLedActive(void *userdata) {
    AddrLatch *latch = (AddrLatch*)userdata;
    return addrLatchGetValue(latch, SPI_CS_ADDRESS) != 0;
}

static bool getMmcClk(void *userdata) {
    AddrLatch *latch = (AddrLatch*)userdata;
    return (addrLatchGetValue(latch, SPI_MOSI_CLK_ADDRESS) & 1) == 1;
}

static bool getMmcCs(void *userdata) {
    AddrLatch *latch = (AddrLatch*)userdata;
    return addrLatchGetValue(latch, SPI_CS_ADDRESS) == 1;
}

static bool getMmcSi(void *userdata) {
    AddrLatch *latch = (AddrLatch*)userdata;
    return (addrLatchGetValue(latch, SPI_MOSI_CLK_ADDRESS) & 2) == 2;
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

    m68kInit(&cpu, rwFunc, &bus, busCheckInterrupt, &bus);
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
        printf("Loading ROM file \"%s\" (%d bytes)\n", args.romFile, (uint32_t)romFileSize);
    } else {
        fprintf(stderr, "ROM file empty or not found: \"%s\"\n", args.romFile);
        return 1;
    }

    uint32_t mmcCapacity = 64 * 1024 * 1024;
    SectorStorage storage;
    sectorStorageInit(&storage, mmcCapacity);

    if (strlen(args.mmcFile) > 0) {
        printf("Loading MMC file \"%s\"\n", args.mmcFile);
        loadFile(args.mmcFile, storage.data, mmcCapacity);
        inject_file(storage.data, "build/blinker.exe");
    }

    ReadWriteMappingKey mappingKey;
    memset(&mappingKey, 0, sizeof(ReadWriteMappingKey));

    const uint32_t uartFreq = 1843200;
    Uart *uart = uartCreate(args.cpuFreq, uartFreq);
    if (uart == NULL) {
        fprintf(stderr, "Failed to initialize UART\n");
        return 1;
    }
    

    mappingKey.start = UART_BASE;
    mappingKey.end = UART_BASE + 0x100000;
    mappingKey.userdata = uart;
    busAddResetFunc(&bus, uartReset, uart);
    busAddClockFunc(&bus, uartClock, uart);
    busAddWriteFunc(&bus, uartWriteByte, uartWriteWord, mappingKey);
    busAddReadFunc(&bus, uartReadByte, uartReadWord, mappingKey);
    busAddInterruptFunc(&bus, 5, uartIsInterrupt, uart);

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

    Spi spi;
    spiInit(&spi, getMmcCs, getMmcSi, getMmcClk, &outReg);
    Mmc mmc;
    mmcInit(&mmc, &spi, sectorStorageReadSector, sectorStorageWriteSector, &storage);
    busAddClockFunc(&bus, mmcClock, &mmc);
    
    Ireg ireg;
    iregInit(&ireg, &mmc);
    mappingKey.start = 0xe00000;
    mappingKey.end = 0xf00000;
    mappingKey.userdata = &ireg;
    mappingKey.conditionFunc = NULL;
    mappingKey.conditionFuncUserdata = NULL;
    busAddReadFunc(&bus, iregReadByte, iregReadWord, mappingKey);

    const int sampleFreq = 48000;
    const int videoFreq = 25175000;
    if (!appInit(&app, &cpu.cpu, busClock, &bus, busReset, &bus, args.cpuFreq, videoFreq, sampleFreq, isLedActive, &outReg)) {
        return 1;
    }
    appRun(&app);
    appDestroy(&app);
    sectorStorageFree(&storage);
    return 0;
}