#pragma once

#include "ticker.h"
#include "readwritefunc.h"
#include "reset.h"
#include "booleanfunc.h"

typedef void (*ClockFunc)(void *user_data, int clocks);

#define MAX_RESET_FUNC 16
#define MAX_CLOCK_FUNC 16
#define MAX_READWRITE_FUNC 16

typedef struct {
    /** Start address, inclusive */
    uint32_t start;
    /** End address, exclusive */
    uint32_t end;
    void *userdata;
    BooleanFunc conditionFunc;
    void *conditionFuncUserdata;
} ReadWriteMappingKey;

typedef struct {
    ReadWriteMappingKey key;
    ReadByteFunc readByteFunc;
    ReadWordFunc readWordFunc;
} ReadMapping;

typedef struct {
    ReadWriteMappingKey key;
    WriteByteFunc writeByteFunc;
    WriteWordFunc writeWordFunc;
} WriteMapping;

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

    WriteMapping writeMappings[MAX_READWRITE_FUNC];
    int writeMappingsCount;
} Bus;


void busInit(Bus *bus);

void busAddCpu(Bus *bus, MainTicker cpuTicker, void *cpuTickerUserdata);

void busAddClockFunc(Bus *bus, ClockFunc clockFunc, void *userdata);

void busAddResetFunc(Bus *bus, ResetFunc resetFunc, void *userdata);

void busAddReadFunc(Bus *bus, ReadByteFunc readByteFunc, ReadWordFunc readWordFunc, ReadWriteMappingKey mappingKey);

void busAddWriteFunc(Bus *bus, WriteByteFunc writeByteFunc, WriteWordFunc writeWordFunc, ReadWriteMappingKey mappingKey);

void busReset(void *userdata);

void busWriteByte(void *userdata, uint32_t address, uint8_t value);

void busWriteWord(void *userdata, uint32_t address, uint16_t value);

uint8_t busReadByte(void *userdata, uint32_t address);

uint16_t busReadWord(void *userdata, uint32_t address);

int busClock(void *userdata);