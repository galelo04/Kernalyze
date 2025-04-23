#ifndef DEFS_H
#define DEFS_H
#include <sys/types.h>

#define MSG_TYPE_PCB 1
#define MSG_QUEUE_KEYFILE "keyfile/key.txt"

typedef enum { READY, RUNNING, FINISHED } PROCESS_STATE;
struct PCB {
    // Data from the file
    int id;
    int arriveTime;
    int priority;
    int runningTime;
    // Data filled by the scheduler
    pid_t pid;
    int remainingTime;
    int waitTime;
    int startTime;
    int finishTime;
    double turnaroundTime;
    double weightedTurnaroundTime;
    PROCESS_STATE state;
};

struct PCBMessage {
    long mtype;
    struct PCB pcb;
};

#endif  // DEFS_H