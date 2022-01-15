#ifndef CONSOLE
#define CONSOLE

typedef struct console {
    int cursorYPos, cursorXPos;
    void (*printString)(const char *message);
    int *(*get_keycode)();
    void (*clear)();
    void (*clearLine)();
    void (*forceCursorTo)(int x, int y);
} Console;

// Global console object
Console *console;

void console_init();
void console_free();

#endif