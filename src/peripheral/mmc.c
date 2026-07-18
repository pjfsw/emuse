#include <string.h>
#include <stdio.h>
#include "mmc.h"

void mmcInit(Mmc *mmc, Spi *spi, SectorFunc readSectorFunc, SectorFunc writeSectorFunc, void *sectorFuncUserdata) {
    memset(mmc, 0, sizeof(Mmc));
    mmc->spi = spi;
    mmc->state = MMC_WAIT_CLOCKS;
    mmc->readSectorFunc = readSectorFunc;
    mmc->writeSectorFunc = writeSectorFunc;
    mmc->sectorFuncUserdata = sectorFuncUserdata;
}

static inline void resetRead(Mmc *mmc) {
    mmc->clocks = 7;
    mmc->input = 0;
    mmc->spi->miso = 1;
}

static void stateWaitByte(Mmc *mmc, uint8_t clk) {
    if (!spiIsCs(mmc->spi)) {
        resetRead(mmc);
        mmc->commandPtr = 0;
        //printf("Reset\n");
        return;
    }

    if (clk == 0) {
        mmc->spi->miso = 1; // We're free now!
        return;
    }   
    mmc->input |= (spiIsMosi(mmc->spi) << mmc->clocks);
    if (mmc->clocks > 0) {
        mmc->clocks--;        
    } else {        
        mmc->lastByte = mmc->input;
        resetRead(mmc);
        //printf("R(%02x) ", mmc->lastByte);
        mmc->state = mmc->nextState;
    }
}

static void initStateWaitByte(Mmc *mmc, MmcState nextState) {
    resetRead(mmc);
    mmc->state = MMC_WAIT_BYTE;
    mmc->nextState = nextState;
}

static void stateWaitClocks(Mmc *mmc, bool clock) {
    bool cs = spiIsCs(mmc->spi);
    if (cs) {
        mmc->clocks = 0;
        return;
    }
    if (clock) {
        mmc->clocks++;
    }
    if (mmc->clocks >= 73) {
        printf("MMC: Initial clock pulses received\n");
        initStateWaitByte(mmc, MMC_WAIT_COMMAND);
    }
}

typedef struct {
    uint64_t command;
    uint16_t response_length;
    int ok_state;
    int error_state;
    uint8_t response[4];
} Command;

static Command expectedCommands[] = {
    {
        .command = 0x400000000095,
        .response_length = 0,
        .response = {0,0,0,0},
        .ok_state = 1,
        .error_state = 0
    },
    {
        .command = 0x48000001aa87,
        .response_length = 4,
        .response = {0,0,0x11,0xaa},
        .ok_state = 2,
        .error_state = 1
    },
    {
        .command = 0x7700000000ff,
        .response_length = 0,
        .response = {0,0,0,0},
        .ok_state = 3,
        .error_state = 2,
    },
    {
        .command = 0x6940000000ff,
        .response_length = 0,
        .response = {0,0,0,0},
        .ok_state = -1,
        .error_state = 2,
    }
};

static inline uint8_t getCommandByte(Mmc *mmc) {
    return (mmc->command >> 40);
}

static inline void writeBit(Mmc *mmc) {
    mmc->spi->miso = (mmc->output >> 7);
}

static void stateWriteByte(Mmc *mmc, uint8_t clk) {
    if (clk) {
        if (mmc->clocks == 0) {
            mmc->state = mmc->nextState;
        } else {
            mmc->clocks--;
            mmc->output <<= 1;
        }
    } else {
        writeBit(mmc);
    }
}

static void initStateWriteByte(Mmc *mmc, MmcState nextState) {
    resetRead(mmc);
    mmc->state = MMC_WRITE_BYTE;
    mmc->nextState = nextState;
    //printf("W(%02x) ", mmc->output);
    writeBit(mmc);
}


static void stateWaitCommand(Mmc *mmc, uint8_t clk) {
    if (clk) {
        return;
    }
    if (mmc->commandPtr == 0 && mmc->lastByte == 0xff) {
        initStateWaitByte(mmc, MMC_WAIT_COMMAND);        
        return;
    }
    mmc->command = (mmc->command << 8) | mmc->lastByte;
    mmc->commandPtr++;
    if (mmc->commandPtr < 6) {
        initStateWaitByte(mmc, MMC_WAIT_COMMAND);
    } else {
        if (mmc->initialised) {
            uint8_t command = getCommandByte(mmc);
            if (command == 0x51) { // Read sector
                uint32_t sector = (mmc->command >> 8) & 0xffffffff;
                mmc->command = 0;        
                mmc->commandPtr = 0;        
                printf("MMC: Read sector %d\n", sector);
                mmc->output = 0;
                mmc->readSectorFunc(sector, mmc->sectorBuffer, mmc->sectorFuncUserdata);
                initStateWriteByte(mmc, MMC_READ_SECTOR_TOKEN);
                return;
            } else if (command == 0x40) {
                // Go back to init state
                mmc->expectedCommand = 0;
                mmc->initialised = false;
            }
        }

        if (mmc->command == expectedCommands[mmc->expectedCommand].command) {
            mmc->output = 0;
            mmc->responsePtr = 0;                        
            mmc->okResponse = true;
        } else {
            mmc->output = 4;
            mmc->expectedCommand = expectedCommands[mmc->expectedCommand].error_state;
            mmc->okResponse = false;
        }
        printf("MMC: Received CMD: %06lx, response %d\n", mmc->command, mmc->output);
        initStateWriteByte(mmc, MMC_AFTER_COMMAND_RESPONSE);
        mmc->commandPtr = 0;
    }
}

static void stateAfterCommandResponse(Mmc *mmc, uint8_t clk) {
    if (!clk) {
        //printf("After command response\n");
        mmc->command = 0;
        if (!mmc->okResponse) {
            initStateWaitByte(mmc, MMC_WAIT_COMMAND);
        } else {
            //printf("Current response progress %d/%d\n", 
                //mmc->response_ptr, expected_commands[mmc->expected_command].response_length);
            if (mmc->responsePtr <= expectedCommands[mmc->expectedCommand].response_length - 1) {
                mmc->output = expectedCommands[mmc->expectedCommand].response[mmc->responsePtr];
                //printf("Next response byte\n");
                initStateWriteByte(mmc, MMC_AFTER_COMMAND_RESPONSE);
                mmc->responsePtr++;
            } else if (expectedCommands[mmc->expectedCommand].ok_state == -1) {
                    printf("MMC: Initialised\n");
                    mmc->initialised = true;
                    initStateWaitByte(mmc, MMC_WAIT_COMMAND);
            } else {
                //printf("Response done\n");
                mmc->expectedCommand = expectedCommands[mmc->expectedCommand].ok_state;
                initStateWaitByte(mmc, MMC_WAIT_COMMAND);
            }
        } 
    }
}

static void stateReadSectorToken(Mmc *mmc, uint8_t clk) {
    if (clk) {
        return;
    }
    mmc->output = 0xFE;
    //printf("Write token\n");
    initStateWriteByte(mmc, MMC_READ_SECTOR);
    mmc->bytesLeft = 512;
}

static void stateReadSector(Mmc *mmc, uint8_t clk) {
    if (clk) {
        return;
    }
    mmc->output = mmc->sectorBuffer[512-mmc->bytesLeft];
    mmc->bytesLeft--;

    if (mmc->bytesLeft > 0) {
        initStateWriteByte(mmc, MMC_READ_SECTOR);
    } else {
        mmc->bytesLeft = 2;
        initStateWriteByte(mmc, MMC_READ_SECTOR_CRC);
    }
}

static void stateReadSectorCrc(Mmc *mmc, uint8_t clk) {
    if (clk) {
        return;
    }
    if (mmc->bytesLeft == 0) {
        mmc->command = 0;        
        mmc->commandPtr = 0;        
        initStateWaitByte(mmc, MMC_WAIT_COMMAND);        
    } else {
        mmc->output = 0x00; // TODO CRC
        mmc->bytesLeft--;
        //printf("Write CRC %d\n", 1-mmc->bytes_left);
        initStateWriteByte(mmc, MMC_READ_SECTOR_CRC);
    }
}


void mmcClock(void *userdata, int clocks) {
    Mmc *mmc = (Mmc*)userdata;    
    bool clock = spiIsClock(mmc->spi);
    if (clock == mmc->lastClock) {
        return;
    }
    switch (mmc->state) {
        case MMC_WAIT_CLOCKS:
            stateWaitClocks(mmc, clock);
            break;
        case MMC_WAIT_BYTE:
            stateWaitByte(mmc, clock);
            break;
        case MMC_WRITE_BYTE:
            stateWriteByte(mmc, clock);
            break;
        case MMC_WAIT_COMMAND:
            stateWaitCommand(mmc, clock);
            break;
        case MMC_AFTER_COMMAND_RESPONSE:
            stateAfterCommandResponse(mmc, clock);
            break;
        case MMC_READ_SECTOR_TOKEN:
            stateReadSectorToken(mmc, clock);
            break;
        case MMC_READ_SECTOR:
            stateReadSector(mmc, clock);
            break;
        case MMC_READ_SECTOR_CRC:
            stateReadSectorCrc(mmc, clock);
            break;            
    }    
    mmc->lastClock = clock;
}

uint8_t mmcGetMiso(Mmc *mmc) {
    return mmc->spi->miso & 1;
}