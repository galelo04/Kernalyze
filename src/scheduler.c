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

int idleTime = 0;  // time scheduler waiting and no process in the ready list

double sumWaiting = 0;
int sumWeightedTurnaround = 0;
int sumWeightedSquared = 0;

void init_scheduler() {
    signal(SIGUSR2, proccessGeneratorSignalHandler);
    pcbTable = createList();
}

void run_scheduler() {
    sync_clk();
    int old_clk = get_clk();
    int currentTime;
    while (1) {
        if ((currentTime = get_clk()) != old_clk) {
            old_clk = currentTime;
        }
    }

    destroy_clk(0);
}

struct PCB *fetchNewProcess() {
    // comunicate with the process generateor to get process data
    key_t key = ftok(MSG_QUEUE_KEYFILE, 1);
    if (key == -1) {
        perror("ftok");
        return NULL;
    }

    int msgqid = msgget(key, 0666);
    if (msgqid == -1) {
        perror("msgget");
        return NULL;
    }
    struct PCBMessage msg;
    if (msgrcv(msgqid, &msg, sizeof(struct PCB), MSG_TYPE_PCB, IPC_NOWAIT) == -1) {
        // No new process
        return NULL;
    }

    struct PCB *pcb = (struct PCB *)malloc(sizeof(struct PCB));
    if (pcb == NULL) {
        perror("malloc");
        return NULL;
    }
    *pcb = msg.pcb;
    pcb->remainingTime = pcb->runningTime;
    pcb->state = READY;
    pcb->startTime = get_clk();
    pcb->finishTime = 0;
    pcb->waitTime = 0;
    pcb->turnaroundTime = 0;
    pcb->weightedTurnaroundTime = 0;

    insertAtFront(pcbTable, *pcb);

    // Debugging
    printf("Process %d is added to the ready list\n", pcb->id);
    printf("Process %d: arriveTime=%d, runningTime=%d, priority=%d\n", pcb->id, pcb->arriveTime,
           pcb->runningTime, pcb->priority);
    printf("Process %d: remainingTime=%d, state=%d, startTime=%d\n", pcb->id, pcb->remainingTime,
           pcb->state, pcb->startTime);
    printf("Process %d: finishTime=%d, waitTime=%d, turnaroundTime=%d\n", pcb->id, pcb->finishTime,
           pcb->waitTime, pcb->turnaroundTime);

    return pcb;
}

pid_t startProcess(struct PCB *pcb) {
    int pid = fork();
    if (pid > 0) pcb->pid = pid;
    // TODO call excevp or something
    return pcb->pid;
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

void proccessGeneratorSignalHandler(int signum) {
    struct PCB *pcb = fetchNewProcess();
    signal(SIGUSR2, proccessGeneratorSignalHandler);
}