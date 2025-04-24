#define _POSIX_SOURCE
#include "scheduler.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

struct ListPCB *pcbTable;
struct NodePCB *currentProcess = NULL;
int totalTime = 0;  // total time scheduler running

int idleTime = 0;  // time scheduler waiting and no process in the ready list

double sumWaiting = 0;
int sumWeightedTurnaround = 0;
int sumWeightedSquared = 0;
int schedulerMsqid = -1;

void init_scheduler() {
    signal(SIGUSR1, processExitSignalHandler);
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
}

void run_scheduler() {
    sync_clk();
    int old_clk = get_clk();
    int currentTime;
    while (1) {
        if ((currentTime = get_clk()) != old_clk) {
            while (fetchProcessFromQueue());
            if (pcbTable->size == 0) {
                idleTime++;
            } else {
                struct PCB *pcb = &pcbTable->head->data;
                if (pcb->state == READY) {
                    // Start the process
                    startProcess(pcb);
                    printf("[Scheduler] Process %d is started\n", pcb->id);
                    pcb->state = RUNNING;
                }
                printf("[Scheduler] Process %d: Remaining time:%d\n", pcb->id,
                       pcb->remainingTime == NULL ? -1 : *pcb->remainingTime);
            }
            old_clk = currentTime;
        }
    }

    destroy_clk(0);
}

int fetchProcessFromQueue() {
    // // comunicate with the process generateor to get process data

    struct PCBMessage msg;
    if (msgrcv(schedulerMsqid, &msg, sizeof(struct PCB), MSG_TYPE_PCB, IPC_NOWAIT) == -1) {
        // No new process
        // printf("[Scheduler] No new process to fetch\n");
        return 1;
    }

    if (msg.pcb.id == -1) {
        printf("[Scheduler] No new process to fetch\n");
        return 0;
    }

    printf("[Scheduler] Process %d is fetched from the queue\n", msg.pcb.id);

    struct PCB *pcb = (struct PCB *)malloc(sizeof(struct PCB));
    if (pcb == NULL) {
        perror("malloc");
        return 0;
    }
    *pcb = msg.pcb;
    pcb->remainingTime = NULL;
    pcb->state = READY;
    pcb->startTime = get_clk();
    pcb->finishTime = 0;
    pcb->waitTime = 0;
    pcb->turnaroundTime = 0;
    pcb->weightedTurnaroundTime = 0;

    insertAtFront(pcbTable, *pcb);

    // Debugging
    printf("[Scheduler] Process %d is added to the ready list\n", pcb->id);
    printf("[Scheduler] Process %d: arriveTime=%d, runningTime=%d, priority=%d\n", pcb->id,
           pcb->arriveTime, pcb->runningTime, pcb->priority);
    return 0;
}

void startProcess(struct PCB *pcb) {
    int pid = fork();
    if (pid == -1) {
        perror("fork");
        return;
    }
    if (pid == 0) {
        // child process

        char idStr[32], runtimeStr[32];

        // Convert id, runningTime and shmKey to strings for execv arguments
        snprintf(idStr, sizeof(idStr), "%d", pcb->id);
        snprintf(runtimeStr, sizeof(runtimeStr), "%d", pcb->runningTime);

        char *args[5];
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
    pcb->pid = pid;
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
}

void resumeProcess(struct PCB *pcb) {
    pcb->state = RUNNING;
    kill(pcb->pid, SIGCONT);
}

void stopProcess(struct PCB *pcb) {
    pcb->state = READY;
    kill(pcb->pid, SIGSTOP);
}

void recrodProcessFinish(struct PCB *pcb, int finishTime) {
    // Detach and remove shared memory
    if (shmdt(pcb->shmAddr) == -1) {
        perror("shmdt");
        return;
    }

    if (shmctl(pcb->shmID, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        return;
    }

    pcb->finishTime = finishTime;
    pcb->state = FINISHED;
    pcb->turnaroundTime = pcb->finishTime - pcb->arriveTime;
    pcb->waitTime = pcb->startTime - pcb->arriveTime;
    if (pcb->waitTime < 0) {
        pcb->waitTime = 0;
    }
    if (pcb->turnaroundTime < 0) {
        pcb->turnaroundTime = 0;
    }
    pcb->weightedTurnaroundTime = pcb->turnaroundTime / (double)pcb->runningTime;

    sumWaiting += pcb->waitTime;
    sumWeightedTurnaround += pcb->weightedTurnaroundTime;
    sumWeightedSquared += pcb->weightedTurnaroundTime * pcb->weightedTurnaroundTime;

    printf("[Schedler] Process %d is finished\n", pcb->id);
    printf("[Schedler] Process %d: finishTime=%d, waitTime=%d, turnaroundTime=%2f\n", pcb->id,
           pcb->finishTime, pcb->waitTime, pcb->turnaroundTime);
    printf("[Schedler] Process %d: weightedTurnaroundTime=%2f\n", pcb->id,
           pcb->weightedTurnaroundTime);
}

void calculatePerformance(int totalTime, int idleTime) {
    // struct NodePCB * current = pcbTable->head;
    // int sumWait = 0 ;
    // double sumWTA = 0;
    // while(current){
    //     sumWait+= current->data.waitTime;
    //     sumWTA+=current->data.weightedTurnaroundTime;
    // }
    // double AWait  = sumWait/pcbTable->size;
    // double AWTA = sumWTA/pcbTable->size;
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
    pid_t pid = wait(NULL);
    if (pid == -1) {
        perror("wait");
        return;
    }
    // Find the PCB of the exited process
    struct NodePCB *current = pcbTable->head;
    while (current != NULL) {
        if (current->data.pid == pid) {
            recrodProcessFinish(&current->data, get_clk());
            break;
        }
        current = current->next;
    }
    signal(SIGUSR1, processExitSignalHandler);
}