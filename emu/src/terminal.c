#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#include "terminal.h"

#define TERMINAL_DEFAULT_FG 7
#define TERMINAL_DEFAULT_BG 0
#define TERMINAL_DEFAULT_ATTR (TERMINAL_DEFAULT_FG | (TERMINAL_DEFAULT_BG << TERMINAL_ATTR_BG_SHIFT))
#define TERMINAL_UNDERLINE_ROW 14

static const uint32_t palette[16] = {
    0x000000FF, // 0  Black
    0xAA0000FF, // 1  Red
    0x00AA00FF, // 2  Green
    0xAA5500FF, // 3  Brown
    0x0000AAFF, // 4  Blue
    0xAA00AAFF, // 5  Magenta
    0x00AAAAFF, // 6  Cyan
    0xAAAAAAFF, // 7  Light gray
    0x555555FF, // 8  Dark gray
    0xFF5555FF, // 9  Bright red
    0x55FF55FF, // 10 Bright green
    0xFFFF55FF, // 11 Yellow
    0x5555FFFF, // 12 Bright blue
    0xFF55FFFF, // 13 Bright magenta
    0x55FFFFFF, // 14 Bright cyan
    0xFFFFFFFF, // 15 White
};

static int clamp(int value, int min, int max) {
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

// Erased cells get the current colors but no underline/reverse/bold
static uint16_t eraseAttr(Terminal *term) {
    return term->cursor.attr & TERMINAL_ATTR_COLOR_MASK;
}

static void eraseCells(Terminal *term, int row, int fromCol, int toCol) {
    uint16_t attr = eraseAttr(term);
    for (int col = fromCol; col <= toCol; col++) {
        term->screen.chars[row][col] = ' ';
        term->screen.attrs[row][col] = attr;
    }
}

static void eraseRows(Terminal *term, int fromRow, int toRow) {
    for (int row = fromRow; row <= toRow; row++) {
        eraseCells(term, row, 0, TERMINAL_COLS - 1);
    }
}

static void copyRow(Terminal *term, int dst, int src) {
    memcpy(term->screen.chars[dst], term->screen.chars[src], TERMINAL_COLS);
    memcpy(term->screen.attrs[dst], term->screen.attrs[src], TERMINAL_COLS * sizeof(uint16_t));
}

// Scroll rows top..bottom up by count lines, new lines at the bottom
static void scrollUp(Terminal *term, int top, int bottom, int count) {
    count = clamp(count, 0, bottom - top + 1);
    for (int row = top; row <= bottom - count; row++) {
        copyRow(term, row, row + count);
    }
    eraseRows(term, bottom - count + 1, bottom);
}

// Scroll rows top..bottom down by count lines, new lines at the top
static void scrollDown(Terminal *term, int top, int bottom, int count) {
    count = clamp(count, 0, bottom - top + 1);
    for (int row = bottom; row >= top + count; row--) {
        copyRow(term, row, row - count);
    }
    eraseRows(term, top, top + count - 1);
}

static void resetScrollRegion(Terminal *term) {
    term->scrollTop = 0;
    term->scrollBottom = TERMINAL_ROWS - 1;
}

static void setCursor(Terminal *term, int x, int y) {
    term->cursor.x = clamp(x, 0, TERMINAL_COLS - 1);
    term->cursor.y = clamp(y, 0, TERMINAL_ROWS - 1);
    term->cursor.wrapPending = false;
}

// Move cursor vertically, stopping at the scroll region margins if inside it
static void moveCursorVertical(Terminal *term, int delta) {
    int y = term->cursor.y;
    int top = (y >= term->scrollTop) ? term->scrollTop : 0;
    int bottom = (y <= term->scrollBottom) ? term->scrollBottom : TERMINAL_ROWS - 1;
    setCursor(term, term->cursor.x, clamp(y + delta, top, bottom));
}

// Index (LF, ESC D): down one line, scroll the region if at its bottom
static void lineFeed(Terminal *term) {
    term->cursor.wrapPending = false;
    if (term->cursor.y == term->scrollBottom) {
        scrollUp(term, term->scrollTop, term->scrollBottom, 1);
    } else if (term->cursor.y < TERMINAL_ROWS - 1) {
        term->cursor.y++;
    }
}

// Reverse index (ESC M): up one line, scroll the region if at its top
static void reverseLineFeed(Terminal *term) {
    term->cursor.wrapPending = false;
    if (term->cursor.y == term->scrollTop) {
        scrollDown(term, term->scrollTop, term->scrollBottom, 1);
    } else if (term->cursor.y > 0) {
        term->cursor.y--;
    }
}

static void putGlyph(Terminal *term, uint8_t c) {
    if (term->cursor.wrapPending) {
        term->cursor.x = 0;
        lineFeed(term);
    }
    term->screen.chars[term->cursor.y][term->cursor.x] = c;
    term->screen.attrs[term->cursor.y][term->cursor.x] = term->cursor.attr;
    if (term->cursor.x == TERMINAL_COLS - 1) {
        // VT100 style: wrap when the next character arrives, not now
        term->cursor.wrapPending = true;
    } else {
        term->cursor.x++;
    }
}

static void processControl(Terminal *term, uint8_t c) {
    switch (c) {
        case '\r':
            term->cursor.x = 0;
            term->cursor.wrapPending = false;
            break;
        case '\n':
        case '\v':
        case '\f':
            lineFeed(term);
            break;
        case '\b':
            if (term->cursor.x > 0) {
                term->cursor.x--;
            }
            term->cursor.wrapPending = false;
            break;
        case '\t':
            term->cursor.x = clamp((term->cursor.x + 8) & ~7, 0, TERMINAL_COLS - 1);
            term->cursor.wrapPending = false;
            break;
        default:
            // BEL and other control characters are ignored
            break;
    }
}

static void sendReply(Terminal *term, const char *str) {
    terminalSend(term, (const uint8_t *)str, strlen(str));
}

static void resetState(Terminal *term) {
    term->cursor.x = 0;
    term->cursor.y = 0;
    term->cursor.attr = TERMINAL_DEFAULT_ATTR;
    term->cursor.wrapPending = false;
    term->savedCursor = term->cursor;
    term->cursorVisible = true;
    term->altScreen = false;
    term->state = TERMINAL_STATE_NORMAL;
    resetScrollRegion(term);
    eraseRows(term, 0, TERMINAL_ROWS - 1);
}

static void setAltScreen(Terminal *term, bool enable) {
    if (enable == term->altScreen) {
        return;
    }
    if (enable) {
        term->altSavedCursor = term->cursor;
        term->savedScreen = term->screen;
        term->altScreen = true;
        eraseRows(term, 0, TERMINAL_ROWS - 1);
    } else {
        term->screen = term->savedScreen;
        term->altScreen = false;
        term->cursor = term->altSavedCursor;
    }
    resetScrollRegion(term);
}

// Parameter n, or def if missing or zero
static int param(Terminal *term, int n, int def) {
    if (n >= term->paramCount || term->params[n] == 0) {
        return def;
    }
    return term->params[n];
}

static void selectGraphicRendition(Terminal *term) {
    uint16_t *attr = &term->cursor.attr;
    if (term->paramCount == 0) {
        *attr = TERMINAL_DEFAULT_ATTR;
        return;
    }
    for (int i = 0; i < term->paramCount; i++) {
        int p = term->params[i];
        if (p == 0) {
            *attr = TERMINAL_DEFAULT_ATTR;
        } else if (p == 1) {
            *attr |= TERMINAL_ATTR_BOLD;
        } else if (p == 4) {
            *attr |= TERMINAL_ATTR_UNDERLINE;
        } else if (p == 7) {
            *attr |= TERMINAL_ATTR_REVERSE;
        } else if (p == 22) {
            *attr &= ~TERMINAL_ATTR_BOLD;
        } else if (p == 24) {
            *attr &= ~TERMINAL_ATTR_UNDERLINE;
        } else if (p == 27) {
            *attr &= ~TERMINAL_ATTR_REVERSE;
        } else if ((p >= 30 && p <= 37) || p == 39 || (p >= 90 && p <= 97)) {
            int fg = (p == 39) ? TERMINAL_DEFAULT_FG : (p >= 90) ? p - 90 + 8 : p - 30;
            *attr = (*attr & ~TERMINAL_ATTR_FG_MASK) | fg;
        } else if ((p >= 40 && p <= 47) || p == 49 || (p >= 100 && p <= 107)) {
            int bg = (p == 49) ? TERMINAL_DEFAULT_BG : (p >= 100) ? p - 100 + 8 : p - 40;
            *attr = (*attr & ~TERMINAL_ATTR_BG_MASK) | (bg << TERMINAL_ATTR_BG_SHIFT);
        }
        // Other attributes (blink, 256 colors, ...) are ignored
    }
}

static void setPrivateMode(Terminal *term, bool enable) {
    for (int i = 0; i < term->paramCount; i++) {
        switch (term->params[i]) {
            case 25:
                term->cursorVisible = enable;
                break;
            case 47:
            case 1047:
            case 1049:
                setAltScreen(term, enable);
                break;
            default:
                break;
        }
    }
}

static void insertChars(Terminal *term, int count) {
    int row = term->cursor.y;
    int x = term->cursor.x;
    count = clamp(count, 1, TERMINAL_COLS - x);
    memmove(&term->screen.chars[row][x + count], &term->screen.chars[row][x], TERMINAL_COLS - x - count);
    memmove(&term->screen.attrs[row][x + count], &term->screen.attrs[row][x], (TERMINAL_COLS - x - count) * sizeof(uint16_t));
    eraseCells(term, row, x, x + count - 1);
    term->cursor.wrapPending = false;
}

static void deleteChars(Terminal *term, int count) {
    int row = term->cursor.y;
    int x = term->cursor.x;
    count = clamp(count, 1, TERMINAL_COLS - x);
    memmove(&term->screen.chars[row][x], &term->screen.chars[row][x + count], TERMINAL_COLS - x - count);
    memmove(&term->screen.attrs[row][x], &term->screen.attrs[row][x + count], (TERMINAL_COLS - x - count) * sizeof(uint16_t));
    eraseCells(term, row, TERMINAL_COLS - count, TERMINAL_COLS - 1);
    term->cursor.wrapPending = false;
}

static void executeCsi(Terminal *term, uint8_t final) {
    if (term->hasIntermediate) {
        return;
    }
    if (term->privateMarker == '?') {
        if (final == 'h') {
            setPrivateMode(term, true);
        } else if (final == 'l') {
            setPrivateMode(term, false);
        }
        return;
    }
    if (term->privateMarker != 0) {
        return;
    }
    int x = term->cursor.x;
    int y = term->cursor.y;
    char reply[32];
    switch (final) {
        case 'A': // CUU
            moveCursorVertical(term, -param(term, 0, 1));
            break;
        case 'B': // CUD
        case 'e': // VPR
            moveCursorVertical(term, param(term, 0, 1));
            break;
        case 'C': // CUF
        case 'a': // HPR
            setCursor(term, x + param(term, 0, 1), y);
            break;
        case 'D': // CUB
            setCursor(term, x - param(term, 0, 1), y);
            break;
        case 'E': // CNL
            moveCursorVertical(term, param(term, 0, 1));
            term->cursor.x = 0;
            break;
        case 'F': // CPL
            moveCursorVertical(term, -param(term, 0, 1));
            term->cursor.x = 0;
            break;
        case 'G': // CHA
        case '`': // HPA
            setCursor(term, param(term, 0, 1) - 1, y);
            break;
        case 'd': // VPA
            setCursor(term, x, param(term, 0, 1) - 1);
            break;
        case 'H': // CUP
        case 'f': // HVP
            setCursor(term, param(term, 1, 1) - 1, param(term, 0, 1) - 1);
            break;
        case 'J': // ED
            switch (param(term, 0, 0)) {
                case 0:
                    eraseCells(term, y, x, TERMINAL_COLS - 1);
                    if (y < TERMINAL_ROWS - 1) {
                        eraseRows(term, y + 1, TERMINAL_ROWS - 1);
                    }
                    break;
                case 1:
                    if (y > 0) {
                        eraseRows(term, 0, y - 1);
                    }
                    eraseCells(term, y, 0, x);
                    break;
                default:
                    eraseRows(term, 0, TERMINAL_ROWS - 1);
                    break;
            }
            term->cursor.wrapPending = false;
            break;
        case 'K': // EL
            switch (param(term, 0, 0)) {
                case 0:
                    eraseCells(term, y, x, TERMINAL_COLS - 1);
                    break;
                case 1:
                    eraseCells(term, y, 0, x);
                    break;
                default:
                    eraseCells(term, y, 0, TERMINAL_COLS - 1);
                    break;
            }
            term->cursor.wrapPending = false;
            break;
        case 'L': // IL
            if (y >= term->scrollTop && y <= term->scrollBottom) {
                scrollDown(term, y, term->scrollBottom, param(term, 0, 1));
                term->cursor.x = 0;
                term->cursor.wrapPending = false;
            }
            break;
        case 'M': // DL
            if (y >= term->scrollTop && y <= term->scrollBottom) {
                scrollUp(term, y, term->scrollBottom, param(term, 0, 1));
                term->cursor.x = 0;
                term->cursor.wrapPending = false;
            }
            break;
        case '@': // ICH
            insertChars(term, param(term, 0, 1));
            break;
        case 'P': // DCH
            deleteChars(term, param(term, 0, 1));
            break;
        case 'X': // ECH
            eraseCells(term, y, x, clamp(x + param(term, 0, 1) - 1, x, TERMINAL_COLS - 1));
            term->cursor.wrapPending = false;
            break;
        case 'S': // SU
            scrollUp(term, term->scrollTop, term->scrollBottom, param(term, 0, 1));
            break;
        case 'T': // SD
            scrollDown(term, term->scrollTop, term->scrollBottom, param(term, 0, 1));
            break;
        case 'm': // SGR
            selectGraphicRendition(term);
            break;
        case 'r': { // DECSTBM
            int top = param(term, 0, 1) - 1;
            int bottom = param(term, 1, TERMINAL_ROWS) - 1;
            bottom = clamp(bottom, 0, TERMINAL_ROWS - 1);
            if (top < bottom) {
                term->scrollTop = top;
                term->scrollBottom = bottom;
                setCursor(term, 0, 0);
            }
            break;
        }
        case 's': // SCOSC
            term->savedCursor = term->cursor;
            break;
        case 'u': // SCORC
            term->cursor = term->savedCursor;
            break;
        case 'n': // DSR
            if (param(term, 0, 0) == 5) {
                sendReply(term, "\x1b[0n");
            } else if (param(term, 0, 0) == 6) {
                snprintf(reply, sizeof(reply), "\x1b[%d;%dR", y + 1, x + 1);
                sendReply(term, reply);
            }
            break;
        case 'c': // DA, answer as a VT100 with advanced video option
            if (param(term, 0, 0) == 0) {
                sendReply(term, "\x1b[?1;2c");
            }
            break;
        default:
            // Unsupported sequence, ignored
            break;
    }
}

static void executeEsc(Terminal *term, uint8_t c) {
    switch (c) {
        case '7': // DECSC
            term->savedCursor = term->cursor;
            break;
        case '8': // DECRC
            term->cursor = term->savedCursor;
            break;
        case 'D': // IND
            lineFeed(term);
            break;
        case 'M': // RI
            reverseLineFeed(term);
            break;
        case 'E': // NEL
            term->cursor.x = 0;
            lineFeed(term);
            break;
        case 'c': // RIS
            resetState(term);
            break;
        default:
            break;
    }
}

static void startCsi(Terminal *term) {
    term->state = TERMINAL_STATE_CSI;
    term->paramCount = 0;
    term->params[0] = 0;
    term->privateMarker = 0;
    term->hasIntermediate = false;
}

static void processCsiByte(Terminal *term, uint8_t c) {
    if (c >= '0' && c <= '9') {
        if (term->paramCount == 0) {
            term->paramCount = 1;
        }
        int *p = &term->params[term->paramCount - 1];
        if (*p < 10000) {
            *p = *p * 10 + (c - '0');
        }
    } else if (c == ';') {
        if (term->paramCount == 0) {
            // Leading ';' means an empty first parameter
            term->paramCount = 1;
        }
        if (term->paramCount < TERMINAL_MAX_PARAMS) {
            term->params[term->paramCount++] = 0;
        }
    } else if (c >= 0x3c && c <= 0x3f) {
        term->privateMarker = c;
    } else if (c >= 0x20 && c <= 0x2f) {
        term->hasIntermediate = true;
    } else if (c >= 0x40 && c <= 0x7e) {
        term->state = TERMINAL_STATE_NORMAL;
        executeCsi(term, c);
    }
}

static void processByte(Terminal *term, uint8_t c) {
    // Control characters are executed in any state, ESC restarts a sequence
    if (c == 0x1b) {
        term->state = TERMINAL_STATE_ESC;
        return;
    }
    if (c < 0x20) {
        if (c == 0x18 || c == 0x1a) {
            // CAN, SUB abort a sequence
            term->state = TERMINAL_STATE_NORMAL;
        } else {
            processControl(term, c);
        }
        return;
    }
    switch (term->state) {
        case TERMINAL_STATE_NORMAL:
            if (c != 0x7f) {
                // 0x80-0xff are shown as their CP437 glyphs
                putGlyph(term, c);
            }
            break;
        case TERMINAL_STATE_ESC:
            if (c == '[') {
                startCsi(term);
            } else if (c == '(' || c == ')' || c == '*' || c == '+' || c == '#') {
                // Character set designation and DEC line attributes are ignored
                term->state = TERMINAL_STATE_ESC_SKIP;
            } else {
                term->state = TERMINAL_STATE_NORMAL;
                executeEsc(term, c);
            }
            break;
        case TERMINAL_STATE_ESC_SKIP:
            term->state = TERMINAL_STATE_NORMAL;
            break;
        case TERMINAL_STATE_CSI:
            processCsiByte(term, c);
            break;
    }
}

bool terminalInit(Terminal *term, const char *devicePath, const uint8_t *glyphs) {
    memset(term, 0, sizeof(Terminal));
    term->devicePath = devicePath;
    term->fd = -1;
    term->glyphs = glyphs;
    term->pixels = calloc(TERMINAL_WIDTH * TERMINAL_HEIGHT, sizeof(uint32_t));
    if (term->pixels == NULL) {
        return false;
    }
    resetState(term);
    return true;
}

void terminalDestroy(Terminal *term) {
    terminalDetach(term);
    free(term->pixels);
    term->pixels = NULL;
}

bool terminalAttach(Terminal *term) {
    if (term->fd >= 0) {
        return true;
    }
    int fd = open(term->devicePath, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        fprintf(stderr, "Terminal: failed to open %s: %s\n", term->devicePath, strerror(errno));
        return false;
    }
    struct termios config;
    if (tcgetattr(fd, &config) < 0) {
        perror("Terminal: failed to get termios config");
    } else {
        // Raw mode, same as cfmakeraw() (not available with _XOPEN_SOURCE)
        config.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
        config.c_oflag &= ~OPOST;
        config.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
        config.c_cflag &= ~(CSIZE | PARENB);
        config.c_cflag |= CS8;
        config.c_cc[VMIN] = 1;
        config.c_cc[VTIME] = 0;
        if (tcsetattr(fd, TCSANOW, &config) < 0) {
            perror("Terminal: failed to set termios config");
        }
    }
    term->fd = fd;
    printf("Terminal attached to %s\n", term->devicePath);
    return true;
}

void terminalDetach(Terminal *term) {
    if (term->fd < 0) {
        return;
    }
    terminalCancelUpload(term);
    close(term->fd);
    term->fd = -1;
    printf("Terminal detached from %s\n", term->devicePath);
}

bool terminalIsAttached(Terminal *term) {
    return term->fd >= 0;
}

static void continueUpload(Terminal *term) {
    if (term->uploadData == NULL) {
        return;
    }
    // Non-blocking: write what fits, the rest waits for the next frame
    int n = write(term->fd, term->uploadData + term->uploadPos, term->uploadSize - term->uploadPos);
    if (n > 0) {
        term->uploadPos += n;
    }
    if (term->uploadPos >= term->uploadSize) {
        printf("Terminal: upload complete, %zu bytes\n", term->uploadSize);
        terminalCancelUpload(term);
    }
}

void terminalUpdate(Terminal *term) {
    if (term->fd < 0) {
        return;
    }
    uint8_t buffer[256];
    int n;
    while ((n = read(term->fd, buffer, sizeof(buffer))) > 0) {
        for (int i = 0; i < n; i++) {
            processByte(term, buffer[i]);
        }
    }
    continueUpload(term);
}

bool terminalStartUpload(Terminal *term, const char *path) {
    if (term->fd < 0) {
        return false;
    }
    terminalCancelUpload(term);
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Terminal: failed to open %s: %s\n", path, strerror(errno));
        return false;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (size <= 0) {
        fclose(file);
        fprintf(stderr, "Terminal: %s is empty\n", path);
        return false;
    }
    uint8_t *data = malloc(size);
    if (data == NULL || fread(data, 1, size, file) != (size_t)size) {
        fclose(file);
        free(data);
        fprintf(stderr, "Terminal: failed to read %s\n", path);
        return false;
    }
    fclose(file);
    term->uploadData = data;
    term->uploadSize = size;
    term->uploadPos = 0;
    printf("Terminal: uploading %s, %ld bytes\n", path, size);
    return true;
}

void terminalCancelUpload(Terminal *term) {
    free(term->uploadData);
    term->uploadData = NULL;
    term->uploadSize = 0;
    term->uploadPos = 0;
}

bool terminalIsUploading(Terminal *term) {
    return term->uploadData != NULL;
}

int terminalUploadProgress(Terminal *term) {
    if (term->uploadSize == 0) {
        return 0;
    }
    return (int)(100 * term->uploadPos / term->uploadSize);
}

void terminalSend(Terminal *term, const uint8_t *data, int length) {
    if (term->fd < 0) {
        return;
    }
    while (length > 0) {
        int n = write(term->fd, data, length);
        if (n <= 0) {
            // Pseudo terminal buffer full, drop the rest
            break;
        }
        data += n;
        length -= n;
    }
}

void terminalTextInput(Terminal *term, const char *text) {
    terminalSend(term, (const uint8_t *)text, strlen(text));
}

static void sendString(Terminal *term, const char *str) {
    terminalSend(term, (const uint8_t *)str, strlen(str));
}

bool terminalKeyDown(Terminal *term, SDL_Keycode key, SDL_Keymod mod) {
    if (mod & (SDL_KMOD_LALT | SDL_KMOD_GUI)) {
        // Reserved for the emulator (Alt+Enter, Alt+Q) and the window manager
        return false;
    }
    if ((mod & SDL_KMOD_CTRL) && !(mod & SDL_KMOD_SHIFT)) {
        // Ctrl+letter and friends give the classic control characters.
        // Ctrl+Shift is reserved for emulator hotkeys.
        uint8_t c;
        if (key >= SDLK_A && key <= SDLK_Z) {
            c = key - SDLK_A + 1;
        } else if (key == SDLK_SPACE || key == SDLK_2) {
            c = 0x00;
        } else if (key == SDLK_LEFTBRACKET) {
            c = 0x1b;
        } else if (key == SDLK_BACKSLASH) {
            c = 0x1c;
        } else if (key == SDLK_RIGHTBRACKET) {
            c = 0x1d;
        } else {
            c = 0xff;
        }
        if (c != 0xff) {
            terminalSend(term, &c, 1);
            return true;
        }
    }
    switch (key) {
        case SDLK_RETURN:
        case SDLK_KP_ENTER:  sendString(term, "\r");      return true;
        case SDLK_BACKSPACE: sendString(term, "\x7f");    return true;
        case SDLK_TAB:       sendString(term, "\t");      return true;
        case SDLK_ESCAPE:    sendString(term, "\x1b");    return true;
        case SDLK_UP:        sendString(term, "\x1b[A");  return true;
        case SDLK_DOWN:      sendString(term, "\x1b[B");  return true;
        case SDLK_RIGHT:     sendString(term, "\x1b[C");  return true;
        case SDLK_LEFT:      sendString(term, "\x1b[D");  return true;
        case SDLK_HOME:      sendString(term, "\x1b[H");  return true;
        case SDLK_END:       sendString(term, "\x1b[F");  return true;
        case SDLK_INSERT:    sendString(term, "\x1b[2~"); return true;
        case SDLK_DELETE:    sendString(term, "\x1b[3~"); return true;
        case SDLK_PAGEUP:    sendString(term, "\x1b[5~"); return true;
        case SDLK_PAGEDOWN:  sendString(term, "\x1b[6~"); return true;
        default:             return false;
    }
}

const uint32_t *terminalRender(Terminal *term, bool showCursor) {
    showCursor = showCursor && term->cursorVisible;
    for (int row = 0; row < TERMINAL_ROWS; row++) {
        for (int col = 0; col < TERMINAL_COLS; col++) {
            const uint8_t *glyph = &term->glyphs[term->screen.chars[row][col] * TERMINAL_CHAR_HEIGHT];
            uint16_t attr = term->screen.attrs[row][col];
            int fgIndex = attr & TERMINAL_ATTR_FG_MASK;
            if ((attr & TERMINAL_ATTR_BOLD) && fgIndex < 8) {
                fgIndex += 8;
            }
            uint32_t fg = palette[fgIndex];
            uint32_t bg = palette[(attr & TERMINAL_ATTR_BG_MASK) >> TERMINAL_ATTR_BG_SHIFT];
            bool reverse = (attr & TERMINAL_ATTR_REVERSE) != 0;
            if (showCursor && row == term->cursor.y && col == term->cursor.x) {
                reverse = !reverse;
            }
            if (reverse) {
                uint32_t tmp = fg;
                fg = bg;
                bg = tmp;
            }
            uint32_t *dst = &term->pixels[row * TERMINAL_CHAR_HEIGHT * TERMINAL_WIDTH + col * TERMINAL_CHAR_WIDTH];
            for (int y = 0; y < TERMINAL_CHAR_HEIGHT; y++) {
                uint8_t bits = glyph[y];
                if (y == TERMINAL_UNDERLINE_ROW && (attr & TERMINAL_ATTR_UNDERLINE)) {
                    bits = 0xff;
                }
                for (int x = 0; x < TERMINAL_CHAR_WIDTH; x++) {
                    dst[x] = (bits & (0x80 >> x)) ? fg : bg;
                }
                dst += TERMINAL_WIDTH;
            }
        }
    }
    return term->pixels;
}
