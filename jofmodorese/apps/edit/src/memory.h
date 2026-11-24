#ifndef MEMORY_H
#define MEMORY_H

#include "arch.h"

// Set count number of bytes to zero from the specified address
void memclr(REG("a0") void *address, REG("d0") int count);

// Copy count number of bytes from src to target. Overlapping behavior is undefined
REG("a1") void *memcopy(REG("a1") void *target, REG("a0") void *src, REG("d0") int count);

#endif