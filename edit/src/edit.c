#include "console.h"
#include "memory.h"
#include "strutils.h"

#define KEY_UP  0x101
#define KEY_DOWN 0x102
#define KEY_RIGHT 0x103
#define KEY_LEFT 0x104

#define MAX_LINE_LENGTH 256
#define MAX_LINES 512

typedef struct {
    char text[MAX_LINE_LENGTH][MAX_LINES];
    int count; 
    // Cursor column
    int col;
    // Actual row in buffer at top of screen
    int top;
    // Cursor row relative to screen top
    int row;
    int width;
    int height;
    int linesHeight;
} Data;

static Data global_data;

static char *getLineAt(Data *data, int i) {
    return data->text[i];
}

static void insertLine(Data *data, char *text) {
    data->count++;
    char *targetLine = getLineAt(data, data->count-1);
    stringCopy(targetLine, text);
}

static void dataInit(Data *data) {
    memclr(data, sizeof(Data));
    char *txt = "Type something";
    int n = 0;
    while (txt[n] != 0) {
        data->text[0][n] = txt[n];
        n++;
    }
    data->text[0][n] = txt[n];
    int max = 100;
    for (int i = 1; i < max; i++) {
        data->text[i][0] = (i%26)+'a';
        data->text[i][1] = 0;
    }
    data->count = max;

//  Needs PEA instruction
//   insertLine(data, "Type something");
//  insertLine(data, "");
}


static int getChar() {
    int key;
    do {
        key = congetc();        
    } while (key == EOF);
    return key;
}

static int readKey() {
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

static void refresh(Data *data) {
    data->width = conwidth();
    data->height = conheight();
    data->linesHeight = data->height - 1;
    conclr();
    consetcrs(data->height,1);
    conreverse();
    for (int i = 0; i < data->width; i++) {
        conputc(' ');
    }
    concrsleft(data->width);
    conputs("/SOMEFILE.TXT");
    consetcrs(data->height,data->width-16);
    conputs("1/???");    
    connormal();
    consetcrs(1,1);
}

static void setEditorCursor(Data *data) {
    consetcrs(data->row+1, data->col+3);
}

// Returns 1 if refresh is needed
static int moveUp(Data *data) {
    if ((data->row == 0) && (data->top == 0)) {
        return 0;
    }

    if (data->row == 0) {
        data->top--;
        return 1;
    } else {
        data->row--;
        concrsup(1);
        return 0;
    }
}

// Returns 1 if refresh is needed
static int moveDown(Data *data) {
    if ((data->row + data->top) == (data->count-1)) {
        return 0;
    }

    if (data->row == data->linesHeight-1) {
        data->top++;
        return 1;
    } else {
        data->row++;
        concrsdown(1);
        return 0;
    }
}

static void redrawLines(Data *data) {
    int row = data->top;
    for (int i = 0; i < data->linesHeight ; i++) {
        if (row >= data->count) {
            break;
        }
        consetcrs(i+1,1);
        conclrline();
        conbold();
        conputc((row%10)+'0');
        conputc(':');
        connormal();
        conputs(getLineAt(data, row));
        row++;
    }
    setEditorCursor(data);
}

static void run() {   
    Data *data = &global_data;
    dataInit(data);
    refresh(data);
    redrawLines(data);
    int running = 1;
    while (running) {
        int key = readKey();
        switch (key) {
            case KEY_UP:
                if (moveUp(data)) {
                    redrawLines(data);
                }
                break;
            case KEY_DOWN:
                if (moveDown(data)) {
                    redrawLines(data);
                }
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
