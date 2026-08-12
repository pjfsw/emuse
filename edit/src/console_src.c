#include "console.h"

int getChar() {
    int key;
    do {
        key = congetc();        
    } while (key == EOF);
    return key;
}

static void getSize(int *w, int *h) {
    int c;
    int row = 0;
    int col = 0;

    consetcrs(99,99);

    conputs("\x1b[6n");

    c = getChar();
    if (c != 27) {
        return;
    }

    c = getChar();
    if (c != '[') {
        return;
    }

    /* Parse row */
    for (;;) {
        c = getChar();

        if (c == ';') {
            break;
        }

        if (c < '0' || c > '9') {
            return;
        }

        row = row * 10 + (c - '0');
    }

    /* Parse column */
    for (;;) {
        c = getChar();

        if (c == 'R') {
            break;
        }

        if (c < '0' || c > '9') {
            return;
        }

        col = col * 10 + (c - '0');
    }

    *h = row;
    *w = col;
}

int conwidth() {
    int w;
    int h;
    getSize(&w, &h);
    return w;
}

int conheight() {
    int w;
    int h;
    getSize(&w, &h);
    return h;
}

static void putNumber(int number) {
    if (number > 9) {
        conputc((number/10)+'0');
    }
    conputc((number%10)+'0');
}

void consetarea(int top, int bottom) {
    conputc(27);
    conputc('[');
    putNumber(top);
    conputc(';');
    putNumber(bottom);
    conputc('r');
}

void conscrollup() {
    conputs("\x1b" "D");
}

void conscrolldown() {
    conputs("\x1b" "M");
}

void concrsoff() {
    conputs("\x1b[?25l");
}

void concrson() {
    conputs("\x1b[?25h");
}

void concrssave() {
    conputs("\x1b""7");
}

void concrsrestore() {
    conputs("\x1b""8");
}

