#include "console_logger.h"

// Define the CONSOLE_LOG_LEVEL enumeration


void printLog(CONSOLE_LOG_LEVEL level, const char* module, const char* format, ...) {
    switch (level) {
        case CONSOLE_LOG_INFO:
            printf("[" ANSI_COLOR_BOLD_BLUE "%s" ANSI_COLOR_RESET "] ", module);
            break;
        case CONSOLE_LOG_ERROR:
            printf("[" ANSI_COLOR_BOLD_RED "%s" ANSI_COLOR_RESET "] ", module);
            break;
        case CONSOLE_LOG_WARNING:
            printf("[" ANSI_COLOR_BOLD_YELLOW "%s" ANSI_COLOR_RESET "] ", module);
            break;
        case CONSOLE_LOG_SUCCESS:
            printf("[" ANSI_COLOR_BOLD_GREEN "%s" ANSI_COLOR_RESET "] ", module);
            break;
        case CONSOLE_LOG_DEBUG:
            printf("[" ANSI_COLOR_BOLD_CYAN "%s" ANSI_COLOR_RESET "] ", module);
            break;
        default:
            printf("[" ANSI_COLOR_BOLD_WHITE "%s" ANSI_COLOR_RESET "] ", module);
            break;
    }

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\n");
    fflush(stdout);
}
// Example usage of the printLog function
// int main() {
//     printLog(CONSOLE_LOG_INFO, "Main", "This is an info message.");
//     printLog(CONSOLE_LOG_ERROR, "Main", "This is an error message.");    
//     printLog(CONSOLE_LOG_WARNING, "Main", "This is a warning message.");
//     printLog(CONSOLE_LOG_SUCCESS, "Main", "This is a success message.");
//     printLog(CONSOLE_LOG_DEBUG, "Main", "This is a debug message.");
//     return 0;
// }