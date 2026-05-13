#pragma once

#include <stdint.h>

typedef uint8_t (*ReadByteFunc)(void *user_data, uint32_t address);
typedef uint16_t (*ReadWordFunc)(void *user_data, uint32_t address);
