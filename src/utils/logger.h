#ifndef LOGGER_H
#define LOGGER_H
#include "../defs.h"

void initLogger();

void logStart(struct PCB* pcb, int currentTime);
void logResume(struct PCB* pcb, int currentTime);
void logStopped(struct PCB* pcb, int currentTime);
void logFinish(struct PCB* pcb, int currentTime, int remainingTime);
void logSchedulerPerformance(int idleTime, int totalTime, int numProcesses, double sumWTA,
                             double sumWTAsquared, int sumWait);
void destroyLogger();

#endif  // LOGGER_H