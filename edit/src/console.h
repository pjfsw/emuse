#ifndef CONSOLE_H
#define CONSOLE_H

#define EOF -1

// Switch to underlined text
extern void conunder();

// Switch to reverse text
extern void conreverse();

// Switch to normal text
extern void connormal();

// Switch to bold text
extern void conbold();

// Clear console
extern void conclr();

// Write string to console
extern void conputs(__reg("a1") const char *str);

// Write char to console
extern void conputc(__reg("d0") int c);

// Get character from console, or EOF if no character
extern int __reg("d0") congetc();

#endif