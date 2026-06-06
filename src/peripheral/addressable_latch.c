#include "addressable_latch.h"
#include <stdio.h>


void addrLatchInit(AddrLatch *latch, uint16_t bitWidth) {
    latch->bitWidth = bitWidth;    
    latch->bitMask = (1<<bitWidth)-1;
}

void addrLatchReset(void *userdata) {
    AddrLatch *latch = (AddrLatch*)userdata;
    
    for (int i = 0 ; i < ADDR_LATCH_MAX_ADDRESS; i++) {
        latch->value[i] = 0;
    }
}

static void writeByte(AddrLatch *latch, uint8_t offset, uint8_t byte) {
    latch->value[offset & (ADDR_LATCH_MAX_ADDRESS-1)] = byte & latch->bitMask;    
}

void addrLatchWriteByte(void *userdata, uint32_t address, uint8_t byte) {
    AddrLatch *latch = (AddrLatch*)userdata;
    //printf("Write byte %06x = %02x\n", address, byte);

    // Write on only odd addresses
    if (!(address & 1)) {
        return; 
    }
    writeByte(latch, address>>1, byte);
}

void addrLatchWriteWord(void *userdata, uint32_t address, uint16_t word) {
    AddrLatch *latch = (AddrLatch*)userdata;

    writeByte(latch,address>>1,(uint8_t)word);
}

uint8_t addrLatchGetValue(AddrLatch *latch, uint32_t address) {
    uint8_t value =  latch->value[(address>>1) & (ADDR_LATCH_MAX_ADDRESS-1)] & latch->bitMask;
    //printf("Value %d\n", value);
    return value;
}