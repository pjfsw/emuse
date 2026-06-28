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
static const int LSR_DR = 0x01;
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
    Fifo rfifo;
    Fifo tfifo;
    int32_t sendTimer;
    int32_t recvTimer;
    uint8_t ier;
    uint8_t lcr;
    uint8_t dll;
    uint8_t dlm;
    uint8_t iir;
    uint8_t dlab;
    uint8_t lsr;
    uint8_t scratch;
    uint8_t fifo_enabled;
    uint8_t fifo_trigger;
    int32_t rxTimeout;    
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

static bool fifoIsFull(Fifo *fifo) {
    return fifo->count == UART_FIFO_SIZE;
}

static bool fifoIsEmpty(Fifo *fifo) {
    return fifo->count == 0;
}

static void fifoClear(Fifo *fifo) {
    fifo->rptr = 0;
    fifo->wptr = 0;
    fifo->count = 0;
}

static bool fifoWrite(Fifo *fifo, uint8_t byte) {
    if (fifoIsFull(fifo)) {
        return false;
    }
    fifo->data[fifo->wptr] = byte;
    fifo->wptr = (fifo->wptr + 1) & UART_FIFO_SIZE_MASK;
    fifo->count++;
    return true;
}

static bool fifoRead(Fifo *fifo, uint8_t *byte) {
    if (fifoIsEmpty(fifo)) {
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

static void transmitByte(Uart *uart, uint8_t byte) {
    if (!fifoWrite(&uart->tfifo, byte)) {
        printf("Transmit discard\n");
    }
}

static void writeByteToUart(Uart *uart, uint8_t offset, uint8_t byte) {
    offset = offset & (uartMaxAddress - 1);
    if ((offset < 2) && (true == uart->dlab)) {
        writeDlab(uart, offset, byte);
        return;
    }
    if (offset == THR) {
        // transmit 
        transmitByte(uart, byte);
    } else if (offset == IER) {
        uart->ier = byte;
        printf("IER = %02x\n", byte);
    } else if (offset == LCR) {
        uart->lcr = byte;
        updateDlab(uart);
        printf("LCR = %02x\n", byte);
    } else if (offset == FCR) {
        setFcr(uart, byte);
    } else if (offset == SCR) {
        printf("Write scratch\n");
        uart->scratch = byte;
    }
}

void uartWriteByte(void *userdata, uint32_t address, uint8_t byte) {
    Uart *uart = (Uart*)userdata;
    if (!(address & 1)) {
        return; 
    }
    writeByteToUart(uart, address>>1, byte);
}

void uartWriteWord(void *userdata, uint32_t address, uint16_t word) {
    Uart *uart = (Uart*)userdata;

    writeByteToUart(uart, address>>1, (uint8_t)word);
}

static uint8_t receiveByte(Uart *uart) {
    if (!fifoIsEmpty(&uart->rfifo)) {
        uint8_t value;
        if (fifoRead(&uart->rfifo, &value)) {
            return value;
        }
    }
    return 0;
}

static uint8_t readByteFromUart(Uart *uart, uint8_t offset) {
    offset = offset & (uartMaxAddress - 1);
    if (offset == RBR) {
        return receiveByte(uart);
    } else if (offset == LSR) {
        return uart->lsr;
    } else if (offset == SCR) {
        printf("Read scratch\n");
        return uart->scratch;
    }
    return 0x00;
}

uint8_t uartReadByte(void *userdata, uint32_t address) {    
    if (!(address & 1)) {
        return 0xaa; 
    }
    Uart *uart = (Uart*)userdata;    
    return readByteFromUart(uart, address>>1);
}

uint16_t uartReadWord(void *userdata, uint32_t address) {
    Uart *uart = (Uart*)userdata;
    return readByteFromUart(uart,address>>1);
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
    if (fifoIsEmpty(&uart->tfifo)) {
        uart->sendTimer = 0;
        return;
    }
    if (uart->sendTimer <= 0) {
        uart->sendTimer += uart->ticksPerCharacter;
    } else {
        uart->sendTimer -= clocks;
        if (uart->sendTimer <= 0) {
            uint8_t byte;
            if (fifoRead(&uart->tfifo, &byte)) {
                ptsWriteByte(uart->pts, byte);
            } else {
                printf("TX BUG! This is not supposed to happen\n");
            }
        }
    }
}

static void handleRxFifo(Uart *uart, int clocks) {
    if (uart->rxTimeout > 0) {
        uart->rxTimeout -= clocks;
    }
    if (fifoIsFull(&uart->rfifo) || !ptsIsByteAvailable(uart->pts)) {
        uart->recvTimer = 0;
        return;
    }
    if ((uart->recvTimer <= 0)) {
        uart->recvTimer += uart->ticksPerCharacter;
    } else {
        uart->recvTimer -= clocks;
        if (uart->recvTimer <= 0) {
            uint8_t byte;
            if (ptsReadByte(uart->pts, &byte)) {
                fifoWrite(&uart->rfifo, byte);
                uart->rxTimeout = 4 * uart->ticksPerCharacter;
            } else {
                printf("RX BUG! This is not supposed to happen\n");
            }
        }
    }        
}

static void handleStatusRegisters(Uart *uart) {
    uart->lsr &= ~(LSR_TEMT | LSR_THRE | LSR_DR);
    
    if (!fifoIsFull(&uart->tfifo)) {
        uart->lsr |= LSR_THRE;
        // TODO not entirely accurate, should immediately poll fifo to a "shift register" and time that
    }
    if (fifoIsEmpty(&uart->tfifo)) {
        uart->lsr |= LSR_TEMT;
    }
    if (!fifoIsEmpty(&uart->rfifo)) {
        uart->lsr |= LSR_DR;
    }
}

void uartClock(void *userdata, int clocks) {
    Uart *uart = (Uart*)userdata;    
    handleTxFifo(uart, clocks);
    handleRxFifo(uart, clocks);
    handleStatusRegisters(uart);
}

bool uartIsInterrupt(void *userdata) {
    Uart *uart = (Uart*)userdata;    

    if ((uart->ier & 0x01) == 0)
        return false;

    if (!uart->fifo_enabled) {
        return !fifoIsEmpty(&uart->rfifo);
    }

    return (uart->rfifo.count > 0 && (uart->rxTimeout <= 0)) || uart->rfifo.count >= uart->fifo_trigger; 
}