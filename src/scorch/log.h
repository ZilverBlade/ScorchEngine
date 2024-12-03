#pragma once
#include <iostream>

void setConsoleColor(int col);

#define SELOG_ERR(fmt, ...)				\
	(setConsoleColor(12));	\
	std::printf("[ERROR]\t"##fmt"\n", __VA_ARGS__);	\
	(setConsoleColor(7))

#define SELOG_WRN(fmt, ...)				\
	(setConsoleColor(14));	\
	std::printf("[WARN]\t"##fmt"\n", __VA_ARGS__);\
	(setConsoleColor(7))

#define SELOG_INF(fmt, ...)				\
	std::printf("[INFO]\t"##fmt"\n", __VA_ARGS__)