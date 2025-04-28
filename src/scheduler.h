#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <sys/types.h>

#include "clk.h"
#include "defs.h"

// handle process generator signal
void fetchProcessFromQueue();
void schedulerClkHandler(int);

// Cleanup resources
void schedulerClearResources(int);

void initScheduler(int type, int quantum);
void runScheduler();

// proccess control
void resumeProcess(struct PCB* pcb);
void stopProcess(struct PCB* pcb);

// handle process exit
void handleProcessExit(struct PCB* pcb);

// scheduler
struct PCB* schedule();
void pushToReadyQueue(struct PCB* pcb);

#endif