#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct Uart Uart;

Uart *uartCreate(uint32_t cpuClockSpeed, uint32_t uartClockSpeed);

uint8_t uartReadByte(void *userdata, uint32_t address);

uint16_t uartReadWord(void *userdata, uint32_t address);

void uartWriteByte(void *userdata, uint32_t address, uint8_t byte);

void uartWriteWord(void *userdata, uint32_t address, uint16_t word);

void uartReset(void *userdata);

void uartClock(void *userdata, int clocks);