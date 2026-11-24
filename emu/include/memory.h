#pragma once

#include <stdint.h>

typedef struct {
    uint8_t *data;
    uint32_t size;
    uint32_t mask;
} Memory;

/**
 * Initialize the memory with the specific size.
 * 
 * Size will be truncated to the nearest power of 2.
 * 
 */
void memoryInit(Memory *memory, uint32_t size);

void memoryFree(Memory *memory);

void memoryWriteByte(void *userdata, uint32_t address, uint8_t byte);

void memoryWriteWord(void *userdata, uint32_t address, uint16_t word);

uint8_t memoryReadByte(void *userdata, uint32_t address);

uint16_t memoryReadWord(void *userdata, uint32_t address);