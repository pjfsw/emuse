#include "bus.h"

#include <string.h>

void busInit(Bus *bus) {
    memset(bus, 0, sizeof(Bus));
}

void busAddCpu(Bus *bus, MainTicker cpuTicker, void *cpuTickerUserdata) {
    bus->cpuTicker = cpuTicker;
    bus->cpuTickerUserdata = cpuTickerUserdata;
}

void busAddClockFunc(Bus *bus, ClockFunc clockFunc, void *userdata) {
    bus->clockFunc[bus->clockFuncCount] = clockFunc;
    bus->clockFuncUserdata[bus->clockFuncCount] = userdata;
    bus->clockFuncCount++;
}

void busAddResetFunc(Bus *bus, ResetFunc resetFunc, void *userdata) {
    bus->resetFunc[bus->resetFuncCount] = resetFunc;
    bus->resetFuncUserdata[bus->resetFuncCount] = userdata;
    bus->resetFuncCount++;
}

void busAddReadFunc(Bus *bus, ReadByteFunc readByteFunc, ReadWordFunc readWordFunc, ReadWriteMappingKey mappingKey) {
    bus->readMappings[bus->readMappingsCount].readByteFunc = readByteFunc;
    bus->readMappings[bus->readMappingsCount].readWordFunc = readWordFunc;
    bus->readMappings[bus->readMappingsCount].key = mappingKey;
    bus->readMappingsCount++;
}

void busReset(Bus *bus) {
    for (int i = 0; i < bus->resetFuncCount; i++) {
        bus->resetFunc[i](bus->resetFuncUserdata[i]);
    }
}

int busClock(void *userdata) {
    Bus *bus = (Bus*)userdata;
    int cycles = bus->cpuTicker(bus->cpuTickerUserdata);
    for (int i = 0; i < bus->clockFuncCount; i++) {
        bus->clockFunc[i]( bus->clockFuncUserdata[i], cycles);
    }
    return cycles;
}

void busWriteByte(Bus *bus, uint32_t address, uint8_t value) {
}

void busWriteWord(Bus *bus, uint32_t address, uint16_t value) {
}

static inline uint8_t getRandomByte() {
    return 0xaa;
}

uint8_t busReadByte(Bus *bus, uint32_t address) {
    for (int i = 0; i < bus->readMappingsCount; i++) {
        ReadMapping *m = &bus->readMappings[i];
        if ((address >= m->key.start) && (address < m->key.end)) {
            if (m->readByteFunc != NULL) {
                return m->readByteFunc(m->key.userdata, address);
            } else {
                return getRandomByte();
            }
        } 
    }
    return getRandomByte();
}

uint16_t busReadWord(Bus *bus, uint32_t address) {
    for (int i = 0; i < bus->readMappingsCount; i++) {
        ReadMapping *m = &bus->readMappings[i];
        if ((address >= m->key.start) && (address < m->key.end)) {
            if (m->readWordFunc != NULL) {
                return m->readWordFunc(m->key.userdata, address);
            } else {
                return getRandomByte();
            }
        } 
    }
    return getRandomByte();
}
