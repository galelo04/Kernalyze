#ifndef LOGGER_H
#define LOGGER_H
#include "../defs.h"

enum LOG_LEVEL {
    LOG_START,
    LOG_RESUME,
    LOG_STOPPED,
    LOG_FINISH,
};

extern const char* LOG_LEVEL_STR[];

void initLogger();

void logProcess(struct PCB* pcb, int currentTime, enum LOG_LEVEL level);
void logSchedulerPerformance(int idleTime, int totalTime, int numProcesses, double sumWTA,
                             double sumWTAsquared, int sumWait);
void destroyLogger();

#endif  // LOGGER_H