#include "sector_storage.h"

#include <stdlib.h>
#include <string.h>

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