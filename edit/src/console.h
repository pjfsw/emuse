#ifndef CONSOLE_H
#define CONSOLE_H

#include "arch.h"

#define EOF -1

// Clear entire line
extern void conclrline(void);

// Clear to end of line
extern void conclreol(void);

// Set cursor position (0..99)
extern void consetcrs(REG("d0") int y, REG("d1") int x);

// Move cursor right (0..99)
extern void concrsright(REG("d0") int steps);

// Move cursor left (0..99)
extern void concrsleft(REG("d0") int steps);

// Move cursor down (0..99)
extern void concrsdown(REG("d0") int steps);

// Move cursor upp (0..99)
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
extern void conputc(REG("d0") unsigned char c);

// Get character from console, or EOF if no character
extern int REG("d0") congetc();

int conwidth();

int conheight();

void consetarea(int top, int bottom);

void conscrollup();

void conscrolldown();

void concrsoff();

void concrson();

void concrssave();

void concrsrestore();

#endif