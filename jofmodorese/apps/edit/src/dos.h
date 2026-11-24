#ifndef DOS_H
#define DOS_H

#include "arch.h"
#include "stdint.h"

typedef struct {
    void *fsPtr;
    uint32_t partitionId;
    void *sectorBufferPtr;
    uint32_t firstSector;
    uint32_t currentBlock;
    uint32_t nextSector;
    uint16_t offset;
    uint32_t bytesRemaining;
    uint8_t attr;
    uint8_t reserved;    
} DosCtx;

/**
 * Create context for existing file or directory
 * 
 * ctx - Pointer to target context
 * path - The path (directory or file)
 * 
 * Return 0 on success or < 0 on error
 */
extern int32_t REG("d0") dosCreateCtx(REG("a0") DosCtx *ctx, REG("a1") const char *path);

/**
 * Read bytes from file
 * 
 * ctx - File context
 * buffer - Preallocated buffer 
 * length - The number of bytes to read
 * 
 * Returns number of bytes read, or 0 if end of file, or negative value if error
 */
extern int32_t REG("d0") dosReadFile(REG("a0") DosCtx *ctx, REG("a1") void *buffer, REG("d0") uint32_t length);

#endif
