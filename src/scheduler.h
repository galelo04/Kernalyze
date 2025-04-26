#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <sys/types.h>

#include "clk.h"
#include "defs.h"

// handle process generator signal
void fetchProcessFromQueue();

// handle process exit signal
void processExitSignalHandler(int signum);

void initScheduler(int type, int quantum);
void runScheduler();

// proccess control
void resumeProcess(struct PCB* pcb);
void stopProcess(struct PCB* pcb);

// calculate performance
void handleProcessExit(struct PCB* pcb);
void calculatePerformance(int totalTime, int idleTime);

struct PCB* schedule();
void pushToReadyQueue(struct PCB* pcb);

#endif