#ifndef CONSOLE_LOGGER_H
#define CONSOLE_LOGGER_H
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "colors.h"

typedef enum {
    CONSOLE_LOG_INFO,
    CONSOLE_LOG_ERROR,
    CONSOLE_LOG_WARNING,
    CONSOLE_LOG_SUCCESS,
    CONSOLE_LOG_DEBUG
} CONSOLE_LOG_LEVEL;

void printLog(CONSOLE_LOG_LEVEL level, const char* module, const char* format, ...);

#endif  // CONSOLE_LOGGER_H