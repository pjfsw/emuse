#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <SDL3/SDL.h>

#define TERMINAL_COLS 80
#define TERMINAL_ROWS 30
#define TERMINAL_CHAR_WIDTH 8
#define TERMINAL_CHAR_HEIGHT 16
#define TERMINAL_WIDTH (TERMINAL_COLS * TERMINAL_CHAR_WIDTH)
#define TERMINAL_HEIGHT (TERMINAL_ROWS * TERMINAL_CHAR_HEIGHT)
#define TERMINAL_MAX_PARAMS 16

// Cell attribute bits
#define TERMINAL_ATTR_FG_MASK    0x000f
#define TERMINAL_ATTR_BG_MASK    0x00f0
#define TERMINAL_ATTR_BG_SHIFT   4
#define TERMINAL_ATTR_COLOR_MASK 0x00ff
#define TERMINAL_ATTR_BOLD       0x0100
#define TERMINAL_ATTR_UNDERLINE  0x0200
#define TERMINAL_ATTR_REVERSE    0x0400

typedef enum {
    TERMINAL_STATE_NORMAL,
    TERMINAL_STATE_ESC,
    TERMINAL_STATE_ESC_SKIP,    // ESC ( x, ESC # x etc, one more byte to ignore
    TERMINAL_STATE_CSI
} TerminalState;

typedef struct {
    int x;
    int y;
    uint16_t attr;
    bool wrapPending;
} TerminalCursor;

typedef struct {
    uint8_t chars[TERMINAL_ROWS][TERMINAL_COLS];
    uint16_t attrs[TERMINAL_ROWS][TERMINAL_COLS];
} TerminalScreen;

typedef struct {
    const char *devicePath;
    int fd;
    const uint8_t *glyphs;      // 256 glyphs, 8x16, one byte per row
    uint32_t *pixels;           // TERMINAL_WIDTH * TERMINAL_HEIGHT, RGBA8888
    TerminalScreen screen;
    TerminalScreen savedScreen; // Main screen while the alternate screen is active
    bool altScreen;
    TerminalCursor cursor;
    TerminalCursor savedCursor; // ESC 7 / ESC 8
    TerminalCursor altSavedCursor; // ?1049
    bool cursorVisible;
    int scrollTop;              // Scroll region, 0-based, inclusive
    int scrollBottom;
    TerminalState state;
    int params[TERMINAL_MAX_PARAMS];
    int paramCount;
    uint8_t privateMarker;      // '?' etc, 0 if none
    bool hasIntermediate;
    uint8_t *uploadData;        // File being sent, NULL if no upload in progress
    size_t uploadSize;
    size_t uploadPos;
} Terminal;

bool terminalInit(Terminal *term, const char *devicePath, const uint8_t *glyphs);

void terminalDestroy(Terminal *term);

// Open the slave side of the pseudo terminal, like picocom does
bool terminalAttach(Terminal *term);

void terminalDetach(Terminal *term);

bool terminalIsAttached(Terminal *term);

// Read and process all bytes available from the pseudo terminal
void terminalUpdate(Terminal *term);

// Send bytes to the pseudo terminal (keyboard direction)
void terminalSend(Terminal *term, const uint8_t *data, int length);

// Send text from an SDL text input event (UTF-8, passed through unchanged like picocom)
void terminalTextInput(Terminal *term, const char *text);

// Translate a key press that does not produce text (Enter, arrows, Ctrl+letter, ...)
// Returns true if the key was sent to the pseudo terminal
bool terminalKeyDown(Terminal *term, SDL_Keycode key, SDL_Keymod mod);

// Start sending a file to the pseudo terminal. It is written as fast as the
// pseudo terminal accepts it, which follows the emulated UART RTS flow control.
bool terminalStartUpload(Terminal *term, const char *path);

void terminalCancelUpload(Terminal *term);

bool terminalIsUploading(Terminal *term);

// Upload progress in percent
int terminalUploadProgress(Terminal *term);

// Render the screen into the pixel buffer and return it
const uint32_t *terminalRender(Terminal *term, bool showCursor);
