#ifndef CONSOLE_LOGGER_H
#define CONSOLE_LOGGER_H
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "colors.h"

void printInfo(const char* module, const char* format, ...);

void printError(const char* module, const char* format, ...);

void printWarning(const char* module, const char* format, ...);

#endif  // CONSOLE_LOGGER_H