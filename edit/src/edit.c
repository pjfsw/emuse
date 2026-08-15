#include "console.h"
#include "memory.h"
#include "strutils.h"
#include "string.h"
#include <stdint.h>

#define KEY_UP          0x101
#define KEY_DOWN        0x102
#define KEY_RIGHT       0x103
#define KEY_LEFT        0x104
#define KEY_HOME        0x105
#define KEY_END         0x106
#define KEY_DELETE      0x107
#define KEY_BACKSPACE   0x108

#define MAX_LINE_LENGTH 256
#define MAX_LINES 999

typedef struct {
    char text[MAX_LINE_LENGTH][MAX_LINES];    
    int16_t freeSlots[MAX_LINES];
    int16_t index[MAX_LINES];
    int freeCount;
    char dummy[MAX_LINE_LENGTH];
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
    int16_t idx = data->index[i];
    if ((idx >= 0) && (idx < MAX_LINES)) {
        return data->text[idx];
    } else {
        return data->dummy;
    }
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

static void freeSlot(Data *data, int16_t slot) {
    data->text[slot][0] = 0;
    data->freeSlots[data->freeCount++] = slot;    
}

static int16_t allocSlot(Data *data) {
    if (data->freeCount == 0) {
        return -1;
    }
    data->freeCount--;
    return data->freeSlots[data->freeCount];
}

// Return slot index or -1 if full
static int16_t addToSlot(Data *data, char *text) {
    int16_t idx = allocSlot(data);
    if (idx < 0) {
        return -1;
    }
    char *targetLine = data->text[idx];
    stringCopy(targetLine, text);
    return idx;
}

static void insertLineAtPos(Data *data, int pos, char *text) {    
    if (data->count >= MAX_LINES) {
        return;
    }
    int16_t idx = addToSlot(data, text);    
    if (idx < 0) {
        return;
    }
    for (int i = data->count; i > pos; i--) {
        data->index[i] = data->index[i - 1];          
    }
    data->index[pos] = idx;
    data->count++;
}

static void appendLine(Data *data,  char *text) {    
    insertLineAtPos(data, data->count, text);
}

static void insertLine(Data *data, char *text) {
    insertLineAtPos(data, data->row+data->top, text);
}

static void dataInit(Data *data) {
    memclr(data, sizeof(Data));
    for (int i = 0; i < MAX_LINES; i++) {
        data->index[i] = -1;
        data->freeSlots[i] = i;
    }
    data->freeCount = MAX_LINES;
    appendLine(data, "Type something");
    appendLine(data, "You are looking at a very very long line that covers many columns and reaches far outside the screen.");
    appendLine(data, "Another line");
    appendLine(data, "");
    appendLine(data, "alpha");
    appendLine(data, "beta");
    appendLine(data, "delta");
    appendLine(data, "omega");
    appendLine(data, "");
    appendLine(data, "many words written on a single line in attempt to make it overflow the width of the editor.");
    appendLine(data, "");
    appendLine(data, "more words1");
    appendLine(data, "more words2");
    appendLine(data, "more words3");
    appendLine(data, "more words4");
    appendLine(data, "");
    appendLine(data, "additional words written on a single line in attempt to make it overflow the width of the editor!");
    appendLine(data, "");
    appendLine(data, "more words5");
    appendLine(data, "more words6");
    appendLine(data, "more words7");
    appendLine(data, "more words8");
    appendLine(data, "");
    appendLine(data, "complete nonsense written on a single line in attempt to make it overflow the width of the editor...");
    appendLine(data, "");
    appendLine(data, "more words9");
    appendLine(data, "more words10");
    appendLine(data, "more words11");
    appendLine(data, "");
    appendLine(data, "more words12");
    appendLine(data, "more words13");
    appendLine(data, "");
    appendLine(data, "more words14");
    appendLine(data, "more words15");
    appendLine(data, "more words16");
    appendLine(data, "");
    appendLine(data, "more words17");
    appendLine(data, "once again, stuff is written on a single line in attempt to make it overflow the width of the editor!");
    appendLine(data, "more words18");
    appendLine(data, "");
    appendLine(data, "more words19");
    appendLine(data, "");
    appendLine(data, "more words20");
    appendLine(data, "");
    appendLine(data, "This is the final line");
    appendLine(data, "");
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
                case '1':
                    if (getChar() == '~')
                        return KEY_HOME;
                case '3':
                    if (getChar() == '~')
                        return KEY_DELETE;
                    break;
            }
        }
        if (c == 'O') {
            c = getChar();
            switch (c) {
                case 'F':
                    return KEY_END;
            }
        }
    }
    if ((c == 8) || (c == 127)) {
        return KEY_BACKSPACE;
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

static void updateStatusLineAndSetCursor(Data *data) {
    concrsoff();
    conreverse();
    consetcrs(data->height,data->width-16);
    conputhex32(sizeof(int));
    conputc('L');
    putNumberPad(data->row+data->top+1, 3);
    conputc('C');    
    putNumberPad(data->col+data->left+1, 3);
    connormal();
    int len = getCurrentLength(data);
    if (data->col > len) {
        data->col = len;
    }
    consetcrs(data->row+1, data->col+1);
    concrson();
}

static void refresh(Data *data) {
    data->width = conwidth();
    data->height = conheight();
    data->linesHeight = data->height - 1;
    concrsoff();
    conclr();
    consetcrs(data->height,1);
    conreverse();
    for (int i = 0; i < data->width; i++) {
        conputc(' ');
    }
    concrsleft(data->width);
    conputs("/SOMEFILE.TXT");
    connormal();
    consetcrs(1,1);
    consetarea(1,data->linesHeight);
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

static int addChar(Data *data, int c) {
    int len = getCurrentLength(data);
    if (len >= MAX_LINE_LENGTH-1) {
        return 0;
    }
    int cur = data->left + data->col;
    char *line = getCurrentLine(data);  
    for (int i = len; i >= cur; i--) {
        line[i+1] = line[i];
    }
    line[cur] = c;
    return 1;
}

static int joinWithPreviousLine(Data *data) {
    int cur = data->top + data->row; 
    if (cur < 1) {
        return 0;
    }
    
    char *line1 = getLineAt(data, cur-1);
    char *line2 = getLineAt(data, cur);

    int len1 = strlen(line1);
    int len2 = strlen(line2);

    if (len1 + len2 >= MAX_LINE_LENGTH) {
        return 0;
    }

    int16_t freedSlot = data->index[cur];

    moveUp(data);
    moveEnd(data);

    strcat(line1, line2);

    for (int i = cur; i < data->count-1; i++) {
        data->index[i] = data->index[i+1];
    }
    data->index[data->count - 1] = -1;
    data->count--;

    freeSlot(data, freedSlot);

    return 1;
}

static int joinWithNextLine(Data *data) {
    int cur = data->top + data->row;

    if (cur >= data->count - 1) {
        return 0;
    }
    char *line1 = getLineAt(data, cur);
    char *line2 = getLineAt(data, cur+1);

    int len1 = strlen(line1);
    int len2 = strlen(line2);

    if ((len1 + len2) >= MAX_LINE_LENGTH) {
        return 0;
    }

    int16_t freedSlot = data->index[cur+1];

    strcat(line1, line2);

    for (int i = cur + 1; i < data->count - 1; i++) {
        data->index[i] = data->index[i + 1];
    }
    data->index[data->count - 1] = -1;
    data->count--;

    freeSlot(data, freedSlot);

    return 1;
}


static int splitLine(Data *data) {
    if (data->count >= MAX_LINES) {
        return 0;
    }
    int curLine = data->top + data->row;
    int curCol = data->left + data->col;
    char *line = getCurrentLine(data);
    insertLineAtPos(data, curLine+1, &line[curCol]);
    line[curCol] = 0;
    return 1;
}

static int deleteLeftChar(Data *data) {
    int cur = data->left + data->col;
    char *line = getCurrentLine(data);  
    int len = getCurrentLength(data);
    for (int i = cur-1; i < len; i++) {
        line[i] = line[i+1];
    }
    return 1;
}

static int deleteChar(Data *data) {
    int cur = data->left + data->col;
    int len = getCurrentLength(data);
    if (cur == len) {
        return 0;
    }
    char *line = getCurrentLine(data);  
    for (int i = cur; i < len; i++) {
        line[i] = line[i+1];
    }
    return 1;
}

static void run() {   
    Data *data = &global_data;
    dataInit(data);
    refresh(data);
    redrawLines(data, 1, data->linesHeight);
    updateStatusLineAndSetCursor(data);

    int running = 1;
    int old;
    while (running) {        
        int row = data->row+1;
        int curCol = data->col+data->left;
        int curRow = data->row+data->top;
        int key = readKey();
        int len;
        if ((key >= 32) && (key < 255)) {
            if (addChar(data, key)) {
                moveRight(data);
                redrawLines(data, row, row);
                updateStatusLineAndSetCursor(data);
            }
            continue;
        }
        // TODO (d8,PC,Xn) relative addressing support required if more cases in switch (vbcc switched to jumptable)
        switch (key) {
            case 3: // CTRL+C
                running = 0;
                break;
            case 13:
                if (splitLine(data)) {
                    data->left = 0;
                    data->col = 0;
                    if (moveDown(data)) {
                        redrawLines(data, data->linesHeight - 1, data->linesHeight);
                    } else {
                        redrawLines(data, data->row, data->linesHeight);
                    }
                    updateStatusLineAndSetCursor(data);
                }
                break;
            case KEY_DELETE:
                len = getCurrentLength(data);
                if (len == curCol) {
                    if (joinWithNextLine(data)) {
                        redrawLines(data, row, data->linesHeight);
                        updateStatusLineAndSetCursor(data);
                    }
                } else if (deleteChar(data)) {
                    redrawLines(data, row, row);
                    updateStatusLineAndSetCursor(data);
                }
                break;
            case KEY_BACKSPACE:
                if (curCol == 0) {
                    if (joinWithPreviousLine(data)) {
                        redrawLines(data, data->row+1, data->linesHeight);
                        updateStatusLineAndSetCursor(data);
                    }
                } else if (deleteLeftChar(data)) {
                    moveLeft(data);
                    redrawLines(data, row, row);
                    updateStatusLineAndSetCursor(data);
                }
                break;
            case KEY_UP:
                if (moveUp(data)) {
                    redrawLines(data, 1, 1);
                }
                updateStatusLineAndSetCursor(data);
                break;
            case KEY_DOWN:
                if (moveDown(data)) {
                    redrawLines(data, data->linesHeight, data->linesHeight);
                }
                updateStatusLineAndSetCursor(data);
                break;
            case KEY_LEFT:
                if (moveLeft(data)) {
                    redrawLines(data, row, row);
                }
                updateStatusLineAndSetCursor(data);               
                break;
            case KEY_RIGHT:
                if (moveRight(data)) {
                    redrawLines(data, row, row);
                }
                updateStatusLineAndSetCursor(data);
                break;
            case KEY_HOME:
                old = data->left + data->col;
                data->left = 0;
                data->col = 0;
                if (old > 0) {
                    redrawLines(data, row, row);
                }
                updateStatusLineAndSetCursor(data);
                break;
            case KEY_END:
                if (moveEnd(data)) {
                    redrawLines(data, row, row);
                }
                updateStatusLineAndSetCursor(data);
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
