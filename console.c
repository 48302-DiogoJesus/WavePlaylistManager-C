#include <stdio.h>
#include <stdlib.h>

#include "console.h"

void console_free() {
    free(console);
}

void console_clear() {
    printf("\e[1;1H\e[2J");
}

void console_clearLine() {
    printf("\33[2K\r");
}

void console_forceCursorTo(int x, int y) {
    printf("\033[%d;%dH", (y), (x));
}

// Simple print function that adds a newline at the end (only for strings)
void console_printString(const char *message)
{
	printf("%s\n", message);
}

/**
* Capture a code from a key pressed in standart input
* @returns a pointer to a buffer holding the code for the pressed key (up to 3 integers)
*/
int *console_get_keycode()
{
	// Will hold 1 or 3 chars (1 if single key; 3 if key combo)
	int *buffer = (int *)calloc(3, 1);
	// Set terminal mode to raw
	system("/bin/stty raw");
	int ch;
	int counter = 0;
	while (1)
	{
		ch = getchar();
		if ((counter == 0 && ch == 27) || (counter == 1 && ch == 91) || counter == 2)
		{
			buffer[counter++] = ch;
		}
		else
		{
			buffer[0] = ch;
			break;
		}
		if (counter == 3)
		{
			break;
		}
	}
	// Restore terminal mode
	system("/bin/stty cooked");
	console->clearLine();
	// Clear current line
	return buffer;
}

void console_init() {
    console = (Console *)malloc(sizeof(Console));
    console->printString = &console_printString;
    console->get_keycode = &console_get_keycode;
    console->clear = &console_clear;
    console->clearLine = &console_clearLine;
    console->forceCursorTo = &console_forceCursorTo;
}