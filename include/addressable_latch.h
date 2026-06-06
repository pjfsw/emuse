#pragma once

#include <stdint.h>

#define ADDR_LATCH_MAX_ADDRESS 8
typedef struct {
    uint8_t value[ADDR_LATCH_MAX_ADDRESS];
    uint16_t bitWidth;
    uint16_t bitMask;
} AddrLatch;

void addrLatchInit(AddrLatch *latch, uint16_t bitWidth);

void addrLatchReset(void *userdata);

void addrLatchWriteByte(void *userdata, uint32_t address, uint8_t byte);

void addrLatchWriteWord(void *userdata, uint32_t address, uint16_t word);

uint8_t addrLatchGetValue(AddrLatch *latch, uint32_t address);