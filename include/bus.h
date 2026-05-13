#pragma once

#include <stdint.h>

#include "ticker.h"

typedef void (*ResetFunc)(void *user_data);
typedef void (*ClockFunc)(void *user_data, int clocks);
typedef uint8_t (*ReadByteFunc)(void *user_data, uint32_t address);
typedef uint16_t (*ReadWordFunc)(void *user_data, uint32_t address);

#define MAX_RESET_FUNC 16
#define MAX_CLOCK_FUNC 16
#define MAX_READWRITE_FUNC 16

typedef struct {
    /** Start address, inclusive */
    uint32_t start;
    /** End address, exclusive */
    uint32_t end;
    void *userdata;
} ReadWriteMappingKey;

typedef struct {
    ReadWriteMappingKey key;
    ReadByteFunc readByteFunc;
    ReadWordFunc readWordFunc;
} ReadMapping;

typedef struct {
    MainTicker cpuTicker;
    void *cpuTickerUserdata;
    
    ResetFunc resetFunc[MAX_RESET_FUNC];
    void *resetFuncUserdata[MAX_RESET_FUNC];
    int resetFuncCount;
    
    ClockFunc clockFunc[MAX_CLOCK_FUNC];
    void *clockFuncUserdata[MAX_CLOCK_FUNC];
    int clockFuncCount;
    
    ReadMapping readMappings[MAX_READWRITE_FUNC];
    int readMappingsCount;
} Bus;


void busInit(Bus *bus);

void busAddCpu(Bus *bus, MainTicker cpuTicker, void *cpuTickerUserdata);

void busAddClockFunc(Bus *bus, ClockFunc clockFunc, void *userdata);

void busAddResetFunc(Bus *bus, ResetFunc resetFunc, void *userdata);

void busAddReadFunc(Bus *bus, ReadByteFunc readByteFunc, ReadWordFunc readWordFunc, ReadWriteMappingKey mappingKey);

void busReset(Bus *bus);

void busWriteByte(Bus *bus, uint32_t address, uint8_t value);

void busWriteWord(Bus *bus, uint32_t address, uint16_t value);

uint8_t busReadByte(Bus *bus, uint32_t address);

uint16_t busReadWord(Bus *bus, uint32_t address);

int busClock(void *userdata);