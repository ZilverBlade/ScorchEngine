#include "log.h"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h> // Include the windows header
static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

void setConsoleColor(int col) {
	SetConsoleTextAttribute(hConsole, col);
}
