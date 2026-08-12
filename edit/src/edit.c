#include "console.h"
#include "memory.h"
#include "strutils.h"
#include "string.h"

#define KEY_UP  0x101
#define KEY_DOWN 0x102
#define KEY_RIGHT 0x103
#define KEY_LEFT 0x104
#define KEY_HOME 0x105
#define KEY_END 0x106

#define MAX_LINE_LENGTH 256
#define MAX_LINES 512

typedef struct {
    char text[MAX_LINE_LENGTH][MAX_LINES];
    int count; 
    // Actual row in buffer at top of screen (0-based)
    int top;
    // Cursor row relative to screen top (0-based)
    int row;
    // Actual position at the left of screen for current line (0-based) 
    int left;
    // Cursor column relative to screen left (0-based)
    int col;
    int width;
    int height;
    int linesHeight;
} Data;

static Data global_data;
static char tmps[MAX_LINE_LENGTH];


static char *getLineAt(Data *data, int i) {
    return data->text[i];
}

static char *getCurrentLine(Data *data) {
    return getLineAt(data, data->top + data->row);
}

static int getLineLength(Data *data, int i) {
    return stringLen(getLineAt(data, i));
}

static int getCurrentLength(Data *data) {
    return getLineLength(data, data->top + data->row);
}

static void insertLine(Data *data, char *text) {
    data->count++;
    char *targetLine = getLineAt(data, data->count-1);
    stringCopy(targetLine, text);
}

static void dataInit(Data *data) {
    memclr(data, sizeof(Data));
    insertLine(data, "Type something");
    insertLine(data, "You are looking at a very very long line that covers many columns and reaches far outside the screen.");
    insertLine(data, "Another line");
    insertLine(data, "");
    insertLine(data, "alpha");
    insertLine(data, "beta");
    insertLine(data, "delta");
    insertLine(data, "omega");
    insertLine(data, "");
    insertLine(data, "many words written on a single line in attempt to make it overflow the width of the editor ");
    insertLine(data, "");
    insertLine(data, "more words1");
    insertLine(data, "more words2");
    insertLine(data, "more words3");
    insertLine(data, "more words4");
    insertLine(data, "");
    insertLine(data, "additional words written on a single line in attempt to make it overflow the width of the editor ");
    insertLine(data, "");
    insertLine(data, "more words5");
    insertLine(data, "more words6");
    insertLine(data, "more words7");
    insertLine(data, "more words8");
    insertLine(data, "");
    insertLine(data, "complete nonsense written on a single line in attempt to make it overflow the width of the editor ");
    insertLine(data, "");
    insertLine(data, "more words9");
    insertLine(data, "more words10");
    insertLine(data, "more words11");
    insertLine(data, "");
    insertLine(data, "more words12");
    insertLine(data, "more words13");
    insertLine(data, "");
    insertLine(data, "more words14");
    insertLine(data, "more words15");
    insertLine(data, "more words16");
    insertLine(data, "");
    insertLine(data, "more words17");
    insertLine(data, "once again, stuff is written on a single line in attempt to make it overflow the width of the editor ");
    insertLine(data, "more words18");
    insertLine(data, "");
    insertLine(data, "more words19");
    insertLine(data, "");
    insertLine(data, "more words20");
    insertLine(data, "");
    insertLine(data, "This is the final line");
    insertLine(data, "");
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
                case 'F': return KEY_END;
                case 'H': return KEY_HOME;
            }
        }
    }
    return c;
}

static void putUInt(unsigned int n)
{
    char buf[6];   
    int i = 0;

    do {
        buf[i++] = '0' + (n % 10);
        n /= 10;
    } while (n);

    while (i) {
        conputc(buf[--i]);
    }
}

static void putNumberPad(int n, int pad)
{
    int digits = 1;

    if (n >= 10) {
        digits++;
    }
    if (n >= 100) {
        digits++;
    }

    putUInt(n);

    while (digits++ < 3) {
        conputc(' ');
    }
}


static void updateStatusLine(Data *data) {
    concrsoff();
    conreverse();
    consetcrs(data->height,data->width-16);
    conputc('L');
    putNumberPad(data->row+data->top+1, 3);
    conputc('C');    
    putNumberPad(data->col+data->left+1, 3);
    connormal();
    concrson();
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
    updateStatusLine(data);
    connormal();
    consetcrs(1,1);
    consetarea(1,data->linesHeight);
}

static void setEditorCursor(Data *data) {
    int len = getCurrentLength(data);
    if (data->col > len) {
        data->col = len;
    }
    consetcrs(data->row+1, data->col+1);
}

static void redrawLines(Data *data, int first, int last) {
    int row = data->top + first -1;
    int cur = data->top + data->row;
    concrsoff();
    connormal();
    for (int i = first; i <= last ; i++) {
        consetcrs(i,1);
        conclrline();
        if (row < data->count) {
            if (row == cur) {
                char *c = getLineAt(data, row); 
                strncpy(tmps, &c[data->left], data->width);
                conputs(tmps);
            } else {
                conputs(getLineAt(data, row));
            }
        }
        row++;
    }
    setEditorCursor(data);
    concrson();
}

static void restoreCurrentLine(Data *data) {
    if (data->left == 0) {
        return;
    }
    data->left = 0;
    redrawLines(data, data->row+1, data->row+1);
}


// Returns 1 if refresh is needed
static int moveUp(Data *data) {
    if ((data->row == 0) && (data->top == 0)) {
        return 0;
    }

    restoreCurrentLine(data);

    if (data->row == 0) {
        data->top--;
        conscrolldown();
        return 1;
    } else {
        data->row--;
        //concrsup(1);
        return 0;
    }
}

static int moveLeft(Data *data) {
    if ((data->col == 0) && (data->left == 0)) {
        return 0;
    }
    
    if (data->col == 0) {
        data->left--;
        return 1;
    } else {
        data->col--;
        //concrsleft(1);
        return 0;
    }
}

// Returns 1 if refresh is needed
static int moveDown(Data *data) {
    if ((data->row + data->top) == (data->count-1)) {
        return 0;
    }
    restoreCurrentLine(data);

    if (data->row == data->linesHeight-1) {
        data->top++;
        conscrollup();
        return 1;
    } else {
        data->row++;
        //concrsdown(1);
        return 0;
    }
}

static int moveRight(Data *data) {
    if ((data->col + data->left) == getCurrentLength(data)) {
        return 0;
    }
    if (data->col == data->width-1) {
        data->left++;
        return 1;
    } else {
        data->col++;
        //concrsright(1);
        return 0;
    }
}

static int moveEnd(Data *data) {
    int len = getCurrentLength(data);

    if ((data->col + data->left) == len) {
        return 0;
    }

    if (len < data->left + data->width) {
        /* End is already visible */
        data->col = len - data->left;
        return 0;
    }

    /* Scroll so end of line is at right edge */
    data->left = len - data->width + 1;
    data->col = data->width - 1;

    return 1;    
}

static void run() {   
    Data *data = &global_data;
    dataInit(data);
    refresh(data);
    redrawLines(data, 1, data->linesHeight);
    int running = 1;
    int old;
    while (running) {
        int row = data->row+1;
        int key = readKey();
        switch (key) {
            case 3: // CTRL+C
                running = 0;
                break;
            case KEY_UP:
                if (moveUp(data)) {
                    redrawLines(data, 1, 1);
                }
                updateStatusLine(data);
                setEditorCursor(data);
                break;
            case KEY_DOWN:
                if (moveDown(data)) {
                    redrawLines(data, data->linesHeight, data->linesHeight);
                }
                updateStatusLine(data);
                setEditorCursor(data);
                break;
            case KEY_LEFT:
                if (moveLeft(data)) {
                    redrawLines(data, row, row);
                }
                updateStatusLine(data);               
                setEditorCursor(data);
                break;
            case KEY_RIGHT:
                if (moveRight(data)) {
                    redrawLines(data, row, row);
                }
                updateStatusLine(data);
                setEditorCursor(data);
                break;
            case KEY_HOME:
                old = data->left + data->col;
                data->left = 0;
                data->col = 0;
                if (old > 0) {
                    redrawLines(data, row, row);
                }
                updateStatusLine(data);
                setEditorCursor(data);
                break;
            case KEY_END:
                if (moveEnd(data)) {
                    redrawLines(data, row, row);
                }
                updateStatusLine(data);
                setEditorCursor(data);
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
