#include "memory.h"
#include <stdlib.h>

void memoryInit(Memory *memory, uint32_t size) {
    int n = 1;
    while (n <= size) {
        n = n << 1;
    }
    n = n >> 1;
    memory->size = n;
    memory->mask = n-1;
    memory->data = malloc(memory->size);
}

void memoryFree(Memory *memory) {
    free(memory->data);
}

void memoryWriteByte(void *userdata, uint32_t address, uint8_t byte) {
    Memory *memory = (Memory*)userdata;
    address = address & memory->mask;
    memory->data[address] = byte;
}

void memoryWriteWord(void *userdata, uint32_t address, uint16_t word) {
    Memory *memory = (Memory*)userdata;
    address = (address & memory->mask) & 0xfffffffe;
    memory->data[address] = (word >> 8);
    memory->data[address+1] = (word & 0xff);
}

uint8_t memoryReadByte(void *userdata, uint32_t address) {
    Memory *memory = (Memory*)userdata;
    address = address & memory->mask;
    return memory->data[address];
}

uint16_t memoryReadWord(void *userdata, uint32_t address) {
    Memory *memory = (Memory*)userdata;
    address = (address & memory->mask) & 0xfffffffe;
    return ((uint16_t)(memory->data[address] << 8) | (uint16_t)memory->data[address+1]);
}
