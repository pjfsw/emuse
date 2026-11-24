#ifndef KERNEL_TYPES_H
#define KERNEL_TYPES_H

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned long  u32;
typedef signed char    s8;
typedef short          s16;
typedef long           s32;

#ifdef __VBCC__
#define REG(r) __reg(r)
#else
#define REG(r)
#endif

#define NULL ((void *)0)

#endif
