#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

FILE* logFile;
FILE* perfFile;
void initLogger() {
    logFile = fopen(LOG_FILE, "w");
    if (logFile == NULL) {
        perror("Error opening log file");
        return;
    }
    fflush(logFile);

    perfFile = fopen(PERFORMANCE_FILE, "w");
    if (perfFile == NULL) {
        perror("Error opening performance file");
        return;
    }
    fflush(perfFile);
}

void logStart(struct PCB* pcb, int currentTime) {
    fprintf(logFile, "At time %d process %d started arr %d total %d remain %d wait %d\n",
            currentTime, pcb->id, pcb->arriveTime, pcb->runningTime, *(pcb->remainingTime),
            pcb->waitTime);
    fflush(logFile);
}

void logResume(struct PCB* pcb, int currentTime) {
    fprintf(logFile, "At time %d process %d resumed arr %d total %d remain %d wait %d\n",
            currentTime, pcb->id, pcb->arriveTime, pcb->runningTime, *(pcb->remainingTime),
            pcb->waitTime);
    fflush(logFile);
}

void logStopped(struct PCB* pcb, int currentTime) {
    fprintf(logFile, "At time %d process %d stopped arr %d total %d remain %d wait %d\n",
            currentTime, pcb->id, pcb->arriveTime, pcb->runningTime, *(pcb->remainingTime),
            pcb->waitTime);
    fflush(logFile);
}
void logFinish(struct PCB* pcb, int currentTime, int remainingTime) {
    fprintf(logFile,
            "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n",
            currentTime, pcb->id, pcb->arriveTime, pcb->runningTime, remainingTime, pcb->waitTime,
            pcb->turnaroundTime, pcb->weightedTurnaroundTime);
    fflush(logFile);
}

void logSchedulerPerformance(int idleTime, int totalTime, int numProcesses, int sumWTA,
                             int sumWTAsquared, int sumWait) {
    // Calculate CPU utilization
    double utilization = (totalTime - idleTime) / (double)totalTime * 100.0;

    // Calculate average waiting time and WTA
    double avgWait = sumWait / (double)numProcesses;
    double avgWTA = sumWTA / (double)numProcesses;

    // Calculate standard deviation of WTA
    double varianceWTA = (sumWTAsquared / numProcesses) - (avgWTA * avgWTA);
    double stddevWTA = sqrt(varianceWTA);

    fprintf(perfFile, "CPU utilization = %.2f%%\n", utilization);
    fprintf(perfFile, "Avg WTA = %.2f\n", avgWTA);
    fprintf(perfFile, "Avg Waiting = %.2f\n", avgWait);
    fprintf(perfFile, "Std WTA = %.2f\n", stddevWTA);
    fflush(perfFile);
}

void destroyLogger() {
    fclose(logFile);
    fclose(perfFile);
}
