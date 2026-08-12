#ifndef ARCH_H
#define ARCH_H

#ifdef __VBCC__
#define REG(r) __reg(r)
#else
#define REG(r)
#endif

#define NULL 0

#endif
