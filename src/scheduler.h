#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "clk.h"
#include "utils.h"

void proccessGeneratorSignalHandler(int signum);

void run_scheduler();

void init_scheduler();

struct PCB* fetchNewProcess();

pid_t startProcess(struct PCB* pcb);

void resumeProcess(struct PCB* pcb);

void stopProcess(struct PCB* pcb);

void recrodProcessFinish(struct PCB* pcb, int finishTime);

void calculatePerformance();

#endif