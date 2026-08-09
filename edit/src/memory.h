#ifndef MEMORY_H
#define MEMORY_H

#include "arch.h"

// Set count number of bytes to zero from the specified address
void memclr(REG("A0") void *address, REG("D0") int count);

#endif