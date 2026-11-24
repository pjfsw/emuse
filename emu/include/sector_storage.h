#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t capacity;
    uint8_t *data;
} SectorStorage;

void sectorStorageInit(SectorStorage *storage, uint32_t capacity);

bool sectorStorageReadSector(uint32_t sector, uint8_t *buffer, void *userdata);

bool sectorStorageWriteSector(uint32_t sector, uint8_t *buffer, void *userdata);

void sectorStorageFree(SectorStorage *storage);