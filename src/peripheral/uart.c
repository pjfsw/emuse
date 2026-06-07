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
static const int LSR_THRE = 0x20;
static const int LSR_TEMT = 0x40;

#define UART_FIFO_SIZE 16
#define UART_FIFO_SIZE_MASK (UART_FIFO_SIZE-1)
typedef struct {
    uint8_t data[UART_FIFO_SIZE];
    uint8_t wptr;
    uint8_t rptr;
    uint8_t count;
} Fifo;

struct Uart {
    uint32_t cpuClockSpeed;
    uint32_t uartClockSpeed;
    uint16_t divisor;
    uint32_t baudRate;
    uint32_t ticksPerCharacter;
    PtsHandler *pts;   
    uint8_t ier;
    uint8_t lcr;
    uint8_t dll;
    uint8_t dlm;
    uint8_t iir;
    uint8_t dlab;
    uint8_t lsr;
    uint8_t fifo_enabled;
    uint8_t fifo_trigger;
    Fifo rfifo;
    Fifo tfifo;
    int32_t sendTimer;
};

Uart *uartCreate(uint32_t cpuClockSpeed, uint32_t uartClockSpeed) {
    Uart *uart = calloc(1, sizeof(Uart));
    uart->cpuClockSpeed = cpuClockSpeed;
    uart->uartClockSpeed = uartClockSpeed;
    if (NULL == (uart->pts = ptsHandlerCreate())) {
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
    uart->ticksPerCharacter = 10 * uart->cpuClockSpeed / uart->baudRate;
    printf("Cycles per character %d\n", uart->ticksPerCharacter);
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

static void fifoClear(Fifo *fifo) {
    fifo->rptr = 0;
    fifo->wptr = 0;
    fifo->count = 0;
}

static bool fifoWrite(Fifo *fifo, uint8_t byte) {
    if (fifo->count == UART_FIFO_SIZE) {
        return false;
    }
    fifo->data[fifo->wptr] = byte;
    fifo->wptr = (fifo->wptr + 1) & UART_FIFO_SIZE_MASK;
    fifo->count++;
    return true;
}

static bool fifoRead(Fifo *fifo, uint8_t *byte) {
    if (fifo->count == 0) {
        return false;
    }
    *byte = fifo->data[fifo->rptr];
    fifo->rptr = (fifo->rptr + 1) & UART_FIFO_SIZE_MASK;
    fifo->count--;
    return true;
}

static void setFcr(Uart *uart, uint8_t byte) {
    uart->fifo_enabled = byte & 1;
    if (byte & 0x02) {
        fifoClear(&uart->rfifo);
    }
    if (byte & 0x04) {
        fifoClear(&uart->tfifo);
    }

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

static void sendByte(Uart *uart, uint8_t byte) {
    if (!fifoWrite(&uart->tfifo, byte)) {
        printf("Transmit discard\n");
    }
    //ptsWriteByte(uart->pts, byte);
}

static void writeByte(Uart *uart, uint8_t offset, uint8_t byte) {
    offset = offset & (uartMaxAddress - 1);
    if ((offset < 2) && (true == uart->dlab)) {
        writeDlab(uart, offset, byte);
        return;
    }
    if (offset == THR) {
        // transmit 
        sendByte(uart, byte);
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

uint8_t readByte(Uart *uart, uint8_t offset) {
    offset = offset & (uartMaxAddress - 1);
    if (offset = LSR) {
        return uart->lsr;
    }
    return 0x00;
}

uint8_t uartReadByte(void *userdata, uint32_t address) {    
    if (!(address & 1)) {
        return 0xaa; 
    }
    Uart *uart = (Uart*)userdata;    
    return readByte(uart, address>>1);
}

uint16_t uartReadWord(void *userdata, uint32_t address) {
    Uart *uart = (Uart*)userdata;
    return readByte(uart,address>>1);
}

void uartReset(void *userdata) {
    Uart *uart = (Uart*)userdata;
    uart->ier = 0;
    uart->iir = 1;
    uart->lcr = 0;
    uart->lsr = 0x60;    
    updateDlab(uart);
}

static void handleTxFifo(Uart *uart, int clocks) {
    if (uart->tfifo.count == 0) {
        uart->sendTimer = 0;
        return;
    }
    if (uart->sendTimer <= 0) {
        uart->sendTimer += uart->ticksPerCharacter;
    } else {
        uart->sendTimer -= clocks;
        if (uart->sendTimer <= 0) {
            uint8_t byte;
            if (!fifoRead(&uart->tfifo, &byte)) {
                printf("This is not supposed to happen\n");
            } else {
                ptsWriteByte(uart->pts, byte);
            }
        }
    }
}

static void handleRxFifo(Uart *uart, int clocks) {
}

static void handleStatusRegisters(Uart *uart) {
    uart->lsr &= ~(LSR_TEMT | LSR_THRE);
    
    if (uart->tfifo.count < UART_FIFO_SIZE) {
        uart->lsr |= LSR_THRE;
        // TODO not entirely accurate, should immediately poll fifo to a "shift register" and time that
    }
    if (uart->tfifo.count == 0) {
        uart->lsr |= LSR_TEMT;
    }
}

void uartClock(void *userdata, int clocks) {
    Uart *uart = (Uart*)userdata;    
    handleTxFifo(uart, clocks);
    handleRxFifo(uart, clocks);
    handleStatusRegisters(uart);
}
