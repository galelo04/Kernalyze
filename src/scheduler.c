#define _POSIX_SOURCE
#include "scheduler.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

struct ListPCB *pcbTable;

int idleTime = 0;  // time scheduler waiting and no process in the ready list

double sumWaiting = 0, sumWeightedTurnaround = 0, sumWeightedSquared = 0;

void init_scheduler() { pcbTable = createList(); }

void run_scheduler() {
    sync_clk();
    // int currentTime = get_clk();
    // TODO implement the scheduler :)
    //  You may split it into multiple files
    // upon termination release the clock resources.

    destroy_clk(0);
}

struct PCB *checkForNewArrivals() {
    // comunicate with the process generateor to get process data

    int id = 0;
    int arriveTime = 0;
    int runTime = 0;
    int priority = 0;
    struct PCB *pcb = (struct PCB *)malloc(sizeof(struct PCB));
    pcb->id = id;
    pcb->arriveTime = arriveTime;
    pcb->remainigTime = runTime;
    pcb->priority = priority;
    pcb->executionTime = 0;
    pcb->state = READY;

    insertAtFront(pcbTable, *pcb);

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
    pcb->turnaroundTime = finishTime - pcb->arriveTime;
    pcb->waitTime = pcb->turnaroundTime - pcb->executionTime;
    pcb->weightedTurnaroundTime = pcb->turnaroundTime / (double)pcb->executionTime;

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
