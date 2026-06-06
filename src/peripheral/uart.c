#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "uart.h"
#include "pts_handler.h"

static const int uartMaxAddress = 8;
static const int RBR = 0;
static const int THR = 0;
static const int IER = 1;
static const int IIR = 2;
static const int FCR = 2;
static const int LCR = 3;
static const int MCR = 4;
static const int LSR = 5;
static const int MSR = 6;
static const int SCR = 7;
static const int DLL = 0;
static const int DLM = 1;

struct Uart {
    uint32_t cpuClockSpeed;
    uint32_t uartClockSpeed;
    uint16_t divisor;
    uint32_t baudRate;
    PtsHandler pts;   
    uint8_t ier;
    uint8_t lcr;
    uint8_t dll;
    uint8_t dlm;
    uint8_t iir;
    uint8_t dlab;
    uint8_t fifo_enabled;
    uint8_t fifo_trigger;
};

Uart *uartCreate(uint32_t cpuClockSpeed, uint32_t uartClockSpeed) {
    Uart *uart = calloc(1, sizeof(Uart));
    uart->cpuClockSpeed = cpuClockSpeed;
    uart->uartClockSpeed = uartClockSpeed;
    if (!ptsHandlerInit(&uart->pts)) {
        free(uart);
        return NULL;
    }    
    return uart;
}

static inline void updateDlab(Uart *uart) {
    uart->dlab = ((uart->lcr & 0x80) > 0) ? 1 : 0;
    printf("Set DLAB = %d\n", uart->dlab);
}

static void updateBaudRate(Uart *uart) {
    uart->divisor = (uint16_t)(uart->dll) | ((uint16_t)(uart->dlm) << 8);
    if (uart->divisor == 0) {
        // illegal value
        uart->baudRate = 0;
    } else {
        uart->baudRate = uart->uartClockSpeed / (16 * uart->divisor);
    }
    printf("Set BAUDRATE to %d bps\n", uart->baudRate);
}

static void writeDlab(Uart *uart, uint8_t offset, uint8_t byte) {
    if (offset == DLL) {
        uart->dll = byte;
//        printf("DLL = %02x\n", byte);
        updateBaudRate(uart);
    } else if (offset == DLM) {
        uart->dlm = byte;
        //printf("DLM = %02x\n", byte);
        updateBaudRate(uart);
    }
}

static void setFcr(Uart *uart, uint8_t byte) {
    uart->fifo_enabled = byte & 1;
    switch (byte >> 6) {
        case 0:
            uart->fifo_trigger = 1;
            break;
        case 1:
            uart->fifo_trigger = 4;
            break;
        case 2:
            uart->fifo_trigger = 8;
            break;
        case 3:
            uart->fifo_trigger = 14;
            break;
    }
    printf("Set fifo enabled: %d, Fifo trigger = %d\n", uart->fifo_enabled, uart->fifo_trigger);
}

static void writeByte(Uart *uart, uint8_t offset, uint8_t byte) {
    offset = offset & (uartMaxAddress - 1);
    if ((offset < 2) && (true == uart->dlab)) {
        writeDlab(uart, offset, byte);
        return;
    }
    if (offset == THR) {
        // transmit 
    } else if (offset == IER) {
        uart->ier = byte;
        printf("IER = %02x\n", byte);
    } else if (offset == LCR) {
        uart->lcr = byte;
        updateDlab(uart);
        printf("LCR = %02x\n", byte);
    } else if (offset == FCR) {
        setFcr(uart, byte);
    }
}

void uartWriteByte(void *userdata, uint32_t address, uint8_t byte) {
    Uart *uart = (Uart*)userdata;
    if (!(address & 1)) {
        return; 
    }
    writeByte(uart, address>>1, byte);
}

void uartWriteWord(void *userdata, uint32_t address, uint16_t word) {
    Uart *uart = (Uart*)userdata;

    writeByte(uart, address>>1, (uint8_t)word);
}

uint8_t uartReadByte(void *userdata, uint32_t address) {    
    return 0;
}

uint16_t uartReadWord(void *userdata, uint32_t address) {
        return 0;
}

void uartReset(void *userdata) {
    Uart *uart = (Uart*)userdata;
    uart->ier = 0;
    uart->iir = 1;
    uart->lcr = 0;
    updateDlab(uart);
}

void uartClock(void *userdata, int clocks) {
}
