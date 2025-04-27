#include "console_logger.h"

void printInfo(const char* module, const char* format, ...) {
    printf("[" ANSI_COLOR_BOLD_BLUE "%s" ANSI_COLOR_RESET "] ", module);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\n");
    fflush(stdout);
}

void printError(const char* module, const char* format, ...) {
    printf("[" ANSI_COLOR_BOLD_RED "%s" ANSI_COLOR_RESET "] ", module);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\n");
    fflush(stdout);
}

void printWarning(const char* module, const char* format, ...) {
    printf("[" ANSI_COLOR_BOLD_YELLOW "%s" ANSI_COLOR_RESET "] ", module);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\n");
    fflush(stdout);
}

void printSuccess(const char* module, const char* format, ...) {
    printf("[" ANSI_COLOR_BOLD_GREEN "%s" ANSI_COLOR_RESET "] ", module);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\n");
    fflush(stdout);
}

void printDebug(const char* module, const char* format, ...) {
    printf("[" ANSI_COLOR_BOLD_CYAN "%s" ANSI_COLOR_RESET "] ", module);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\n");
    fflush(stdout);
}