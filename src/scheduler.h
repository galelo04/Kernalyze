#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <sys/types.h>

#include "clk.h"
#include "defs.h"

// handle process generator signal
int fetchProcessFromQueue();

// handle process exit signal
void processExitSignalHandler(int signum);

void init_scheduler();
void run_scheduler();

// proccess control
void startProcess(struct PCB* pcb);
void resumeProcess(struct PCB* pcb);
void stopProcess(struct PCB* pcb);

// calculate performance
void handleProcessExit(pid_t pid);
void calculatePerformance();

void schedule();

#endif