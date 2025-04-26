#define _POSIX_SOURCE
#include "scheduler.h"

#include <errno.h>
#include <fcntl.h>  // for fcntl
#include <math.h>   // for standard deviation calculation
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "defs.h"
#include "utils/circularQueue.h"
#include "utils/console_logger.h"
#include "utils/list.h"
#include "utils/minheap.h"

struct List *pcbTable;
struct PCB *currentProcess = NULL;
int totalTime = 0;  // total time scheduler running

int idleTime = 0;  // time scheduler waiting and no process in the ready list

double sumWaiting = 0;
int sumWeightedTurnaround = 0;
int sumWeightedSquared = 0;
int schedulerMsqid = -1;

// Scheduler parameters
int schedulerType = 0;     // 0: RR, 1: SRTN, 2: HPF
int schedulerQuantum = 1;  // quantum time for RR
int noMoreProcesses = 0;   // no more processes arriving

void *readyQueue = NULL;
int remainingQuantum = 0;  // remaining quantum for current process

void initScheduler(int type, int quantum) {
    // Set scheduler parameters
    schedulerType = type;
    schedulerQuantum = quantum;
    pcbTable = createList();

    // Create message queue for communication with process generator
    key_t key = ftok(MSG_QUEUE_KEYFILE, 1);
    if (key == -1) {
        perror("[Scheduler] ftok");
        exit(EXIT_FAILURE);
    }

    schedulerMsqid = msgget(key, IPC_CREAT | 0666);
    if (schedulerMsqid == -1) {
        perror("[Scheduler] msgget");
        exit(EXIT_FAILURE);
    }

    // Init queue
    if (schedulerType == 0) {  // RR
        readyQueue = (void *)createQueue();
    } else if (schedulerType == 1) {  // SRTN
        readyQueue = (void *)heap_create();
    } else if (schedulerType == 2) {  // HPF
        readyQueue = (void *)heap_create();
    }
}

void runScheduler() {
    syncClk();
    int oldClk = getClk();
    int currentClk;

    while (1) {
        currentClk = getClk();
        if (currentClk != oldClk) {
            // Check for arrived processes
            fetchProcessFromQueue();

            // Update remaining time for running process
            if (currentProcess != NULL) {
                (*currentProcess->remainingTime)--;
                remainingQuantum--;

                // Check for process completion
                int isFinished = 0;
                if (*currentProcess->remainingTime <= 0) {
                    handleProcessExit(currentProcess);
                    currentProcess = NULL;
                    isFinished = 1;
                }

                // Expired quantum
                if (remainingQuantum <= 0 && !isFinished) {
                    pushToReadyQueue(currentProcess);

                    struct PCB *nextProcess = schedule();
                    if (nextProcess != NULL && nextProcess != currentProcess) {
                        stopProcess(currentProcess);
                        currentProcess = nextProcess;
                        resumeProcess(currentProcess);
                        remainingQuantum = schedulerQuantum;
                    }
                }
            }

            // Schedule next process if none is running
            if (currentProcess == NULL) {
                struct PCB *nextProcess = schedule();
                if (nextProcess != NULL) {
                    currentProcess = nextProcess;
                    resumeProcess(currentProcess);
                    remainingQuantum = schedulerQuantum;
                }
            }

            // Check if all processes are finished
            if (noMoreProcesses && currentProcess == NULL &&
                ((schedulerType == 0 && isEmpty((struct Queue *)readyQueue)) ||
                 (schedulerType != 0 && heap_is_empty((struct Heap *)readyQueue)))) {
                printInfo("Scheduler", "No more processes and ready queue is empty, terminating");
                break;
            }

            oldClk = currentClk;
        }
    }
}

struct PCB *schedule() {
    struct PCB *nextProcess = NULL;
    if (schedulerType == 0) {
        // Round robin
        struct Queue *RRreadyQueue = (struct Queue *)readyQueue;
        if (isEmpty(RRreadyQueue)) {
            return NULL;
        } else {
            while (!isEmpty(RRreadyQueue)) {
                nextProcess = (struct PCB *)dequeue(RRreadyQueue);
                if (nextProcess->state == READY) {
                    return nextProcess;
                } else if (currentProcess->state == FINISHED) {
                    continue;
                }
            }
            return NULL;
        }
    } else if (schedulerType == 1) {
        // SRTN
    }
    return NULL;
}

void fetchProcessFromQueue() {
    if (noMoreProcesses) return;

    struct PCBMessage msg;

    while (msgrcv(schedulerMsqid, &msg, sizeof(struct PCB), MSG_TYPE_PCB, 0) != -1) {
        // No processes to fetch for this clock cycle
        if (msg.pdata.id == -1) return;

        // No more processes at all
        if (msg.pdata.id == -2) {
            noMoreProcesses = 1;
            printInfo("Scheduler", "Received signal: no more processes will arrive");
            continue;
        }

        // Create a new PCB and add it to the PCB table
        struct PCB *pcb = (struct PCB *)malloc(sizeof(struct PCB));
        if (pcb == NULL) {
            perror("malloc");
            return;
        }

        // Data from generator
        pcb->id = msg.pdata.id;
        pcb->arriveTime = msg.pdata.arriveTime;
        pcb->runningTime = msg.pdata.runningTime;
        pcb->priority = msg.pdata.priority;
        pcb->pid = msg.pdata.pid;

        // default values
        pcb->state = READY;
        pcb->startTime = 0;
        pcb->finishTime = 0;
        pcb->waitTime = 0;
        pcb->turnaroundTime = 0;
        pcb->weightedTurnaroundTime = 0;

        // Create shared memory for remaining time
        pcb->shmKey = ftok(SHM_KEYFILE, pcb->id);
        if (pcb->shmKey == -1) {
            perror("[Scheduler] ftok");
            return;
        }

        pcb->shmID = shmget(pcb->shmKey, sizeof(int), IPC_CREAT | 0666);
        if (pcb->shmID == -1) {
            perror("[Scheduler] shmget");
            return;
        }

        pcb->shmAddr = shmat(pcb->shmID, NULL, 0);
        if (pcb->shmAddr == (void *)-1) {
            perror("[Scheduler] shmat");
            return;
        }

        pcb->remainingTime = (int *)pcb->shmAddr;
        *pcb->remainingTime = pcb->runningTime;

        insertAtFront(pcbTable, (void *)pcb);
        printInfo("Scheduler", "Process %d arrived at time %d with PID %d", pcb->id,
                  pcb->arriveTime, pcb->pid);

        pushToReadyQueue(pcb);
    }
}

void resumeProcess(struct PCB *pcb) {
    if (pcb->state == FINISHED) return;

    pcb->state = RUNNING;
    kill(pcb->pid, SIGCONT);
    printWarning("Scheduler", "At time %d process %d resumed arr %d total %d remain %d wait %d",
                 getClk(), pcb->id, pcb->arriveTime, pcb->runningTime, *pcb->remainingTime,
                 pcb->waitTime);
}

void stopProcess(struct PCB *pcb) {
    if (pcb->state == FINISHED) return;

    pcb->state = READY;
    kill(pcb->pid, SIGSTOP);
    printWarning("Scheduler", "At time %d process %d stopped arr %d total %d remain %d wait %d",
                 getClk(), pcb->id, pcb->arriveTime, pcb->runningTime, *pcb->remainingTime,
                 pcb->waitTime);
}

void handleProcessExit(struct PCB *pcb) {
    if (pcb->state == FINISHED) return;

    // wait message from processGenerator to tell us that the process has finished
    struct PCBMessage msg;
    int status = msgrcv(schedulerMsqid, &msg, sizeof(struct PCB), MSG_TYPE_TERMINATION, 0);

    if (status == -1) {
        perror("[Scheduler] msgrcv");
        return;
    }

    // update PCB
    pcb->state = FINISHED;
    pcb->finishTime = getClk();
    pcb->turnaroundTime = pcb->finishTime - pcb->arriveTime;
    pcb->weightedTurnaroundTime = (double)pcb->turnaroundTime / pcb->runningTime;

    sumWaiting += pcb->waitTime;
    sumWeightedTurnaround += pcb->weightedTurnaroundTime;
    sumWeightedSquared += pcb->weightedTurnaroundTime * pcb->weightedTurnaroundTime;
    totalTime += pcb->runningTime;

    printWarning("Scheduler", "At time %d process %d finished arr %d total %d remain %d wait %d",
                 getClk(), pcb->id, pcb->arriveTime, pcb->runningTime, *pcb->remainingTime,
                 pcb->waitTime);
}

void calculatePerformance(int totalTime, int idleTime) {
    double cpuUtilization = 100 * (totalTime - idleTime / (double)totalTime);

    // write to the scheduler.pref
    FILE *fp = fopen("scheduler.pref", "w");
    if (fp == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "CPU Utilization: %.2f%%\n", cpuUtilization);
}

void pushToReadyQueue(struct PCB *pcb) {
    if (schedulerType == 0) {
        // Round Robin
        struct Queue *RRreadyQueue = (struct Queue *)readyQueue;
        enqueue(RRreadyQueue, (void *)pcb);
    } else if (schedulerType == 1) {
        // SJF
        struct Heap *SJFreadyQueue = (struct Heap *)readyQueue;
        heap_insert(SJFreadyQueue, (void *)pcb, *pcb->remainingTime);
    } else if (schedulerType == 2) {
        // Priority
        struct Heap *HPFreadyQueue = (struct Heap *)readyQueue;
        heap_insert(HPFreadyQueue, (void *)pcb, pcb->priority);
    }
}