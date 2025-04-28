#include "logger.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

FILE* logFile;
FILE* perfFile;
const char* LOG_LEVEL_STR[] = {
    "started",
    "resumed",
    "stopped",
    "finished",
};
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

void logProcess(struct PCB* pcb, int currentTime, enum LOG_LEVEL level) {
    fprintf(logFile, "At time %d process %d %s arr %d total %d remain %d wait %d", currentTime,
            pcb->id, LOG_LEVEL_STR[level], pcb->arriveTime, pcb->runningTime, *(pcb->remainingTime),
            pcb->waitTime);
    if (level == LOG_FINISH) {
        fprintf(logFile, " TA %d WTA %.2f", pcb->turnaroundTime, pcb->weightedTurnaroundTime);
    }
    fprintf(logFile, "\n");
    fflush(logFile);
}

void logSchedulerPerformance(int idleTime, int totalTime, int numProcesses, double sumWTA,
                             double sumWTAsquared, int sumWait) {
    // Calculate CPU utilization
    double utilization = ((totalTime - idleTime) / (double)totalTime) * 100.0;

    // Calculate average waiting time and WTA
    double avgWTA = sumWTA / numProcesses;
    double avgWait = (double)sumWait / numProcesses;

    // Calculate standard deviation of WTA
    double varianceWTA = (sumWTAsquared / numProcesses) - (avgWTA * avgWTA);
    double stddevWTA = sqrt(varianceWTA);

    fprintf(perfFile,
            "CPU utilization = %.2f%%\nAvg WTA = %.2f\nAvg Waiting = %.2f\nStd WTA = %.2f\n",
            utilization, avgWTA, avgWait, stddevWTA);
    fflush(perfFile);
}

void destroyLogger() {
    fclose(logFile);
    fclose(perfFile);
}
