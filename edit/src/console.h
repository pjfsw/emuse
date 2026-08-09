#ifndef CONSOLE_H
#define CONSOLE_H

#ifdef __VBCC__
#define REG(r) __reg(r)
#else
#define REG(r)
#endif

#define EOF -1

extern void concrsright(REG("d0") int steps);

extern void concrsleft(REG("d0") int steps);

extern void concrsdown(REG("d0") int steps);

extern void concrsup(REG("d0") int steps);

// Switch to underlined text
extern void conunder(void);

// Switch to reverse text
extern void conreverse(void);

// Switch to normal text
extern void connormal(void);

// Switch to bold text
extern void conbold(void);

// Clear console
extern void conclr(void);

// Write string to console
extern void conputs(REG("a1") const char *str);

// Write char to console
extern void conputc(REG("d0") int c);

// Get character from console, or EOF if no character
extern int REG("d0") congetc();

#endif