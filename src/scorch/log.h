#pragma once
#include <iostream>
#include <windows.h> // Include the windows header
static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

#define SELOG_ERR(fmt, fmtdat)				\
	(SetConsoleTextAttribute(hConsole, 12));	\
	std::printf("[ERROR]\t"##fmt"\n", fmtdat);	\
	(SetConsoleTextAttribute(hConsole, 7))

#define SELOG_WRN(fmt, fmtdat)				\
	(SetConsoleTextAttribute(hConsole, 14));	\
	std::printf("[WARN]\t"##fmt"\n", fmtdat);\
	(SetConsoleTextAttribute(hConsole, 7))

#define SELOG_INF(fmt, fmtdat)				\
	std::printf("[INFO]\t"##fmt"\n", fmtdat)