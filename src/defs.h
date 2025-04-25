#ifndef DEFS_H
#define DEFS_H
#include <sys/types.h>
#include <sys/ipc.h>

#define MSG_TYPE_PCB 1
#define MSG_QUEUE_KEYFILE "keyfolder/queueKey.txt"
#define SHM_KEYFILE "keyfolder/shmKey.txt"
#define SEM_KEYFILE "keyfolder/semKey.txt"
#define PROCESS_PATH "./process"

// INITAL: not started yet
// READY: stopped and ready to run
// RUNNING: currently running
// FINISHED: finished executing
typedef enum { INITIAL, READY, RUNNING, FINISHED } PROCESS_STATE;
struct PCB {
    // Data from the file
    int id;
    int arriveTime;
    int priority;
    int runningTime;
    // Data filled by the scheduler
    pid_t pid;
    int* remainingTime;
    int waitTime;
    int startTime;
    int finishTime;
    double turnaroundTime;
    double weightedTurnaroundTime;
    PROCESS_STATE state;

    // Shared memory
    key_t shmKey;
    int shmID;
    void* shmAddr;
};
struct ProcessData {
    int id;
    int arriveTime;
    int runningTime;
    int priority;
};

struct PCBMessage {
    long mtype;
    struct ProcessData pdata;
};

#endif  // DEFS_H