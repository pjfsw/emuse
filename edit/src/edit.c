#include "console.h"
#include "memory.h"

#define KEY_UP  0x101
#define KEY_DOWN 0x102
#define KEY_RIGHT 0x103
#define KEY_LEFT 0x104

#define MAX_LINE_LENGTH 256
#define MAX_LINES 256

typedef struct {
    char lines[MAX_LINE_LENGTH][MAX_LINES];
    int lineCount;
} Data;

static Data global_data;

void dataInit(Data *data) {
    memclr(data, sizeof(Data));
}

char *getLine(Data *data, int i) {
    return data->lines[i];
}

int getChar() {
    int key;
    do {
        key = congetc();        
    } while (key == EOF);
    return key;
}

int readKey() {
    int c = getChar();
    
    if (c == 27) {
        c = getChar();
        if (c == '[') {
            c = getChar();

            switch (c) {
                case 'A': return KEY_UP;
                case 'B': return KEY_DOWN;
                case 'C': return KEY_RIGHT;
                case 'D': return KEY_LEFT;
            }
        }
    }
    return c;
}

int run() {   
    Data *data = &global_data;
    dataInit(data);
    conclr();
    consetcrs(25,1);
    conreverse();
    for (int i = 0; i < 80; i++) {
        conputc(' ');
    }
    concrsleft(80);
    conputs("/SOMEFILE.TXT");
    consetcrs(25,70);
    conputs("1/???");    
    connormal();
    consetcrs(1,1);
    int lines=8;
    const char *text[]={
        "This is some bogus text",
        "to test out the appearance of my sick text editor",
        "bleh",
        "as asdadsjadsjoidsaiojdsajiosadioajdsaidojsdisoj",
        "ls /",
        "if a=b then",
        "  load coolr driver",
        "fi"
    };
    for (int i = 0; i < lines; i++) {
        if (i > 0) {
            conputs("\r\n");
        }
        conbold();
        conputc(i+'0');
        conputc(':');
        connormal();
        conputs(text[i]);
    }
    int running = 1;
    while (running) {
        int key = readKey();
        switch (key) {
            case KEY_UP:
                concrsup(1);
                break;
            case KEY_DOWN:
                concrsdown(1);
                break;
            case 3: // CTRL+C
                running = 0;
                break;
            default:            
        } 
    }

    conclrline();
    concrsleft(80);
}

int main() {
    conputs("\x1b[?1049h");
    run();
    conputs("\x1b[?1049l");
    return 0;
}
