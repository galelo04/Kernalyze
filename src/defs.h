#ifndef DEFS_H
#define DEFS_H
#include <sys/types.h>

typedef enum { READY, RUNNING, FINISHED } PROCESS_STATE;
struct PCB {
    pid_t pid;
    int id, priority, arriveTime, remainigTime, waitTime, executionTime, startTime, finishTime;
    double turnaroundTime, weightedTurnaroundTime;
    PROCESS_STATE state;
};

#endif