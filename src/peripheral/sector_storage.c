#include "sector_storage.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void sectorStorageInit(SectorStorage *storage, uint32_t capacity) {
    storage->capacity = capacity;
    storage->data = calloc(capacity, 1);
}

bool sectorStorageReadSector(uint32_t sector, uint8_t *buffer, void *userdata) {
    SectorStorage *storage = (SectorStorage*)userdata;
    if ((sector * 512 + 511) >= storage->capacity) {
        return false;
    }

    memcpy(buffer, &storage->data[sector*512], 512);

    printf("Copy sector %d contents\n", sector);
    /*for (int i = 0; i < 512; i += 16) {
        printf("%03X: ", i);
        for (int j = 0; j < 16; j++) {
            printf("%02X ", buffer[i + j]);
        }
        printf("\n");
    }*/


    return true;
}

bool sectorStorageWriteSector(uint32_t sector, uint8_t *buffer, void *userdata) {
    SectorStorage *storage = (SectorStorage*)userdata;
    if ((sector * 512 + 511) >= storage->capacity) {
        return false;
    }
    memcpy(&storage->data[sector*512], buffer, 512);
    return true;
}

void sectorStorageFree(SectorStorage* storage) {
    free(storage->data);
}