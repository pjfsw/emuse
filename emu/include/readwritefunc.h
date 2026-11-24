#pragma once

#include <stdint.h>

typedef uint8_t (*ReadByteFunc)(void *user_data, uint32_t address);
typedef uint16_t (*ReadWordFunc)(void *user_data, uint32_t address);
typedef void (*WriteByteFunc)(void *user_data, uint32_t address, uint8_t byte);
typedef void (*WriteWordFunc)(void *user_data, uint32_t address, uint16_t word);

typedef struct {
    ReadByteFunc rb;
    ReadWordFunc rw;
    WriteByteFunc wb;
    WriteWordFunc ww;
} RwFunc;
