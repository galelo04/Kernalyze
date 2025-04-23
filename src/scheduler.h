#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "clk.h"
#include "utils.h"

void run_scheduler();

void init_schuduler();

struct PCB* checkForNewArrivals();

pid_t startProcess(struct PCB* pcb);

void resumeProcess(struct PCB* pcb);

void stopProcess(struct PCB* pcb);

void recrodProcessFinish(struct PCB* pcb, int finishTime);

void calculatePerformance();

#endif