#define _POSIX_SOURCE
#include "scheduler.h"
#include "defs.h"
#include "utils/list.h"
#include "utils/circularQueue.h"
#include "utils/console_logger.h"
#include "utils/minheap.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>  // for fcntl
#include <errno.h>

struct List *pcbTable;
struct PCB *currentProcess = NULL;
int totalTime = 0;  // total time scheduler running

int idleTime = 0;  // time scheduler waiting and no process in the ready list

double sumWaiting = 0;
int sumWeightedTurnaround = 0;
int sumWeightedSquared = 0;
int schedulerMsqid = -1;

// Scheduler parameters
int schedulerType = 0;     // 0: RR, 1: SJF, 2: Priority
int schedulerQuantum = 1;  // quantum time for RR

void *readyQueue = NULL;
int remainingQuantum = 0;  // remaining quantum for current process

int signalPipe[2];

void init_scheduler(int type, int quantum) {
    // Signal handler for process exit
    if (pipe(signalPipe) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    fcntl(signalPipe[0], F_SETFL, O_NONBLOCK);

    struct sigaction sa;
    sa.sa_handler = processExitSignalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NOCLDSTOP;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    schedulerType = type;
    pcbTable = createList();
    // Create message queue
    key_t key = ftok(MSG_QUEUE_KEYFILE, 1);
    if (key == -1) {
        perror("[Sheduler] ftok");
        exit(EXIT_FAILURE);
    }

    schedulerMsqid = msgget(key, IPC_CREAT | 0666);
    if (schedulerMsqid == -1) {
        perror("[Sheduler] msgget");
        exit(EXIT_FAILURE);
    }
    if (schedulerType == 0) {
        readyQueue = (void *)createQueue();
        schedulerQuantum = quantum;
    } else if (schedulerType == 1) {
        readyQueue = (void *)heap_create();
    }
}

void run_scheduler() {
    sync_clk();
    int oldClk = get_clk();
    int currentClk;

    while (1) {
        currentClk = get_clk();
        if (currentClk != oldClk) {
            fetchProcessFromQueue();
            pid_t pid;
            ssize_t bytesRead;
            bytesRead = read(signalPipe[0], &pid, sizeof(pid_t));
            if (bytesRead == sizeof(pid_t)) {
                handleProcessExit(pid);
                currentProcess = NULL;
                remainingQuantum = 0;
            }

            if (currentProcess != NULL) {
                (*currentProcess->remainingTime)--;
                remainingQuantum--;
                if (remainingQuantum <= 0) {
                    pushToReadyQueue(currentProcess);
                    remainingQuantum = 0;

                    struct PCB *nextProcess = schedule();
                    if (nextProcess != NULL && nextProcess != currentProcess) {
                        stopProcess(currentProcess);
                        currentProcess = nextProcess;
                        if (currentProcess->state == INITIAL) {
                            startProcess(currentProcess);
                        } else {
                            resumeProcess(currentProcess);
                        }
                        remainingQuantum = schedulerQuantum;
                    }
                }
            }

            if (currentProcess == NULL) {
                struct PCB *nextProcess = schedule();
                if (nextProcess != NULL) {
                    currentProcess = nextProcess;
                    if (currentProcess->state == INITIAL) {
                        startProcess(currentProcess);
                    } else {
                        resumeProcess(currentProcess);
                    }
                    remainingQuantum = schedulerQuantum;
                }
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
                if (nextProcess->state == INITIAL || nextProcess->state == READY) {
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
    struct PCBMessage msg;
    while (msgrcv(schedulerMsqid, &msg, sizeof(struct PCB), MSG_TYPE_PCB, 0) != -1) {
        // No processes to fetch
        if (msg.pdata.id == -1) {
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

        // default values
        pcb->state = INITIAL;
        pcb->startTime = 0;
        pcb->finishTime = 0;
        pcb->waitTime = 0;
        pcb->turnaroundTime = 0;
        pcb->weightedTurnaroundTime = 0;
        pcb->pid = -1;

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
        printInfo("Scheduler", "Process %d arrived at time %d", pcb->id, pcb->arriveTime);

        pushToReadyQueue(pcb);
    }
}

void startProcess(struct PCB *pcb) {
    int pid = fork();
    if (pid == -1) {
        perror("[Scheduler] fork");
        return;
    }

    if (pid == 0) {
        // child process
        char idStr[32], runtimeStr[32];

        // Convert id and runningTime to strings for execv arguments
        snprintf(idStr, sizeof(idStr), "%d", pcb->id);
        snprintf(runtimeStr, sizeof(runtimeStr), "%d", pcb->runningTime);

        char *args[4];
        args[0] = PROCESS_PATH;
        args[1] = idStr;
        args[2] = runtimeStr;
        args[3] = NULL;
        // Execute the process
        execv(PROCESS_PATH, args);
        perror("[Scheduler] execv");
        exit(EXIT_FAILURE);
    }
    // parent process
    pcb->state = RUNNING;
    pcb->pid = pid;
    pcb->startTime = get_clk();
    pcb->waitTime = pcb->startTime - pcb->arriveTime;
    printf("At time %d process %d started arr %d total %d remain %d wait %d\n", get_clk(), pcb->id,
           pcb->arriveTime, pcb->runningTime, *pcb->remainingTime, pcb->waitTime);
}

void resumeProcess(struct PCB *pcb) {
    if (pcb->state == FINISHED) {
        return;
    }
    pcb->state = RUNNING;
    kill(pcb->pid, SIGCONT);
    printf("At time %d process %d resumed arr %d total %d remain %d wait %d\n", get_clk(), pcb->id,
           pcb->arriveTime, pcb->runningTime, *pcb->remainingTime, pcb->waitTime);
}

void stopProcess(struct PCB *pcb) {
    if (pcb->state == FINISHED) {
        return;
    }
    pcb->state = READY;
    kill(pcb->pid, SIGSTOP);
    printf("At time %d process %d stopped arr %d total %d remain %d wait %d\n", get_clk(), pcb->id,
           pcb->arriveTime, pcb->runningTime, *pcb->remainingTime, pcb->waitTime);
}

void handleProcessExit(pid_t pid) {
    struct Node *node = pcbTable->head;
    struct PCB *pcb = NULL;
    while (node) {
        if (node->data != NULL) {
            pcb = (struct PCB *)node->data;
            if (pcb->pid == pid) {
                break;
            }
        }
        node = node->next;
    }
    if (pcb == NULL) {
        return;
    }
    // TODO: see why this is not working
    // Detach shared memory
    // if (shmdt(pcb->shmAddr) == -1) {
    //     perror("[Scheduler] shmdt");
    //     printf("Error code: %d\n", errno);  // Print the error code
    //     // exit(EXIT_FAILURE);
    // }
    if (shmctl(pcb->shmID, IPC_RMID, NULL) == -1) {
        perror("[Scheduler] shmctl");
        exit(EXIT_FAILURE);
    }

    // Process finished
    pcb->state = FINISHED;
    pcb->finishTime = get_clk() - 1;  // -1 because of the clock sync
    pcb->turnaroundTime = pcb->finishTime - pcb->arriveTime;
    pcb->weightedTurnaroundTime = (double)pcb->turnaroundTime / pcb->runningTime;
    sumWaiting += pcb->waitTime;
    sumWeightedTurnaround += pcb->weightedTurnaroundTime;
    sumWeightedSquared += (pcb->weightedTurnaroundTime * pcb->weightedTurnaroundTime);
    totalTime += pcb->runningTime;
    printf("At time %d process %d finished arr %d total %d remain %d wait %d TA %.2f WTA %.2f\n",
           get_clk(), pcb->id, pcb->arriveTime, pcb->runningTime, *pcb->remainingTime,
           pcb->waitTime, pcb->turnaroundTime, pcb->weightedTurnaroundTime);
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

void processExitSignalHandler(__attribute__((unused)) int signum) {
    // Get the process id of the exited process
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        write(signalPipe[1], &pid, sizeof(pid_t));
    }
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