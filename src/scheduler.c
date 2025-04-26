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
#include "utils/semaphore.h"
#include "utils/logger.h"

struct List *pcbTable;
struct PCB *currentProcess = NULL;

// Note that they start at -1 to be the same as the clk
// As the clk start with -1 to send SIGUSR2 to the scheduler at time 0
int totalTime = -1;  // total time scheduler running
int idleTime = -1;   // time scheduler waiting and no process in the ready list

// Performance metrics
int sumWait = 0;
int sumWTA = 0;
int sumWTAsquared = 0;
int numPocesses = 0;

int schedulerMsqid = -1;
// Scheduler parameters
int schedulerType = 0;     // 0: RR, 1: SRTN, 2: HPF
int schedulerQuantum = 1;  // quantum time for RR
int noMoreProcesses = 0;   // no more processes arriving

void *readyQueue = NULL;
int remainingQuantum = 0;  // remaining quantum for current process
int clkChanged = 0;
int schedulerSemid = -1;
int currentClk;

void schedulerClkHandler(int) {
    currentClk = getClk();
    up(schedulerSemid);
    signal(SIGUSR2, schedulerClkHandler);
}

void initScheduler(int type, int quantum) {
    schedulerSemid = initSemaphore(1);
    initLogger();
    signal(SIGUSR2, schedulerClkHandler);
    signal(SIGINT, schedulerClearResources);
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

    while (1) {
        down(schedulerSemid);

        totalTime++;

        printError("Scheduler", "CurrentClk: %d", currentClk);
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
                }
                remainingQuantum = schedulerQuantum;
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

        if (currentProcess == NULL) {
            // No process is running, increment idle time
            idleTime++;
        }

        // Check if all processes are finished
        if (noMoreProcesses && currentProcess == NULL &&
            ((schedulerType == 0 && isEmpty((struct Queue *)readyQueue)) ||
             (schedulerType != 0 && heap_is_empty((struct Heap *)readyQueue)))) {
            printInfo("Scheduler", "No more processes and ready queue is empty, terminating");
            break;
        }
    }
    // Calculate performance metrics
    logSchedulerPerformance(idleTime, totalTime, numPocesses, sumWTA, sumWTAsquared, sumWait);
    printSuccess("Scheduler", "TotalTime: %d, IdleTime: %d, Processes: %d", totalTime, idleTime,
                 numPocesses);
    raise(SIGINT);
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

    while (1) {
        int status = msgrcv(schedulerMsqid, &msg, sizeof(struct PCB), MSG_TYPE_PCB, 0);
        if (status == -1) {
            if (errno == EINTR) continue;
            perror("[Scheduler] msgrcv");
            raise(SIGINT);
        }

        // No processes to fetch for this clock cycle
        if (msg.pdata.id == -1) return;

        // No more processes at all
        if (msg.pdata.id == -2) {
            noMoreProcesses = 1;
            printInfo("Scheduler", "Received signal: no more processes will arrive");
            return;
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
        pcb->startTime = -1;
        pcb->finishTime = -1;
        pcb->waitTime = -1;
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
        numPocesses++;
    }
}

void resumeProcess(struct PCB *pcb) {
    if (pcb->state == FINISHED) return;

    pcb->state = RUNNING;
    if (pcb->startTime == -1) {
        pcb->startTime = currentClk;
        pcb->waitTime = pcb->startTime - pcb->arriveTime;
        logStart(pcb, currentClk);
        printWarning("Scheduler", "At time %d process %d started arr %d total %d remain %d wait %d",
                     currentClk, pcb->id, pcb->arriveTime, pcb->runningTime, *pcb->remainingTime,
                     pcb->waitTime);
    } else {
        logResume(pcb, currentClk);
        printWarning("Scheduler", "At time %d process %d resumed arr %d total %d remain %d wait %d",
                     currentClk, pcb->id, pcb->arriveTime, pcb->runningTime, *pcb->remainingTime,
                     pcb->waitTime);
    }
    kill(pcb->pid, SIGCONT);
}

void stopProcess(struct PCB *pcb) {
    if (pcb->state == FINISHED) return;

    pcb->state = READY;
    logStopped(pcb, currentClk);
    printWarning("Scheduler", "At time %d process %d stopped arr %d total %d remain %d wait %d",
                 currentClk, pcb->id, pcb->arriveTime, pcb->runningTime, *pcb->remainingTime,
                 pcb->waitTime);
    kill(pcb->pid, SIGSTOP);
}

void handleProcessExit(struct PCB *pcb) {
    if (pcb->state == FINISHED) return;

    // wait message from processGenerator to tell us that the process has finished
    struct PCBMessage msg;
    while (1) {
        int status = msgrcv(schedulerMsqid, &msg, sizeof(struct PCB), MSG_TYPE_TERMINATION, 0);
        if (status == -1) {
            if (errno == EINTR) continue;
            perror("[Scheduler] msgrcv");
            raise(SIGINT);
        }
        break;
    }

    // Save the remaining time and free the shared memory
    int remainingTime = *pcb->remainingTime;

    if (shmdt(pcb->shmAddr) == -1) {
        perror("[Scheduler] shmdt");
        exit(EXIT_FAILURE);
    }

    if (shmctl(pcb->shmID, IPC_RMID, NULL) == -1) {
        perror("[Scheduler] shmctl");
        exit(EXIT_FAILURE);
    }

    // update PCB
    pcb->state = FINISHED;
    pcb->finishTime = currentClk;
    pcb->turnaroundTime = pcb->finishTime - pcb->arriveTime;
    pcb->weightedTurnaroundTime = (double)pcb->turnaroundTime / pcb->runningTime;

    // update performance metrics
    sumWait += pcb->waitTime;
    sumWTA += pcb->weightedTurnaroundTime;
    sumWTAsquared += pcb->weightedTurnaroundTime * pcb->weightedTurnaroundTime;

    logFinish(pcb, currentClk, remainingTime);

    printWarning(
        "Scheduler",
        "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n",
        currentClk, pcb->id, pcb->arriveTime, pcb->runningTime, remainingTime, pcb->waitTime,
        pcb->turnaroundTime, pcb->weightedTurnaroundTime);
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

void schedulerClearResources(int) {
    printInfo("Scheduler", "Scheduler terminating");
    destroySemaphore(schedulerSemid);
    destroyLogger();
    if (msgctl(schedulerMsqid, IPC_RMID, NULL) == -1) {
        perror("[Scheduler] msgctl");
        exit(EXIT_FAILURE);
    }
    exit(0);
}