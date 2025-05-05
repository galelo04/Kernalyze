#ifndef DEFS_H
#define DEFS_H
#include <sys/ipc.h>
#include <sys/types.h>

#define MSG_TYPE_PCB 1
#define MSG_TYPE_TERMINATION 2
#define MSG_QUEUE_KEYFILE "keyfolder/queueKey.txt"
#define SHM_KEYFILE "keyfolder/shmKey.txt"
#define SEM_KEYFILE "keyfolder/semKey.txt"
#define PROCESS_PATH "./process"
#define LOG_FILE "scheduler.log"
#define PERFORMANCE_FILE "scheduler.perf"
#define MEMORY_LOG_FILE "memory.log"
#define SCHEDULER_SEMAPHORE 1
#define PG_SEMAPHORE 2
#define PG_SCHEDULER_SEMAPHORE 3

#define TOTAL_MEMORY 1024
#define MAX_PROCESS_MEMORY 256

// READY: stopped and ready to run
// RUNNING: currently running
// FINISHED: finished executing
typedef enum { READY, RUNNING, FINISHED } PROCESS_STATE;
struct PCB {
    // Data from the file
    int id;
    int arriveTime;
    int priority;
    int runningTime;
    pid_t pid;
    int memsize;
    // Data filled by the scheduler
    int* remainingTime;
    int waitTime;
    int startTime;
    int finishTime;
    int turnaroundTime;
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
    pid_t pid;
    int memsize;
};

struct PCBMessage {
    long mtype;
    struct ProcessData pdata;
};

static inline int comparePCB(void* a, void* b) { return a - b; }

#endif  // DEFS_H