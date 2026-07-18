#pragma once

#include "spi.h"
#include "booleanfunc.h"

typedef enum {
    MMC_WAIT_CLOCKS,
    MMC_WAIT_BYTE,
    MMC_WRITE_BYTE,
    MMC_WAIT_COMMAND,
    MMC_AFTER_COMMAND_RESPONSE,
    MMC_READ_SECTOR_TOKEN,
    MMC_READ_SECTOR,
    MMC_READ_SECTOR_CRC
} MmcState;

typedef bool (*SectorFunc)(uint32_t sector, uint8_t *buffer, void *userdata);

typedef struct {
    Spi *spi;
    SectorFunc readSectorFunc;
    SectorFunc writeSectorFunc;
    void *sectorFuncUserdata;
    uint8_t sectorBuffer[512];    
    uint64_t command;
    int clocks;
    int commandPtr;
    MmcState state;
    MmcState nextState;
    uint8_t input; 
    uint8_t output;
    uint8_t lastByte;  
    uint8_t expectedCommand; 
    uint8_t responsePtr;
    uint16_t bytesLeft;
    bool initialised;
    bool okResponse;
    bool lastClock;
} Mmc;

void mmcInit(Mmc *mmc, Spi *spi, SectorFunc readSectorFunc, SectorFunc writeSectorFunc, void *sectorFuncUserdata);

void mmcClock(void *userdata, int clocks);

uint8_t mmcGetMiso(Mmc *mmc);