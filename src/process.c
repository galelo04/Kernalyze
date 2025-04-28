#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#include "clk.h"
#include "defs.h"
#include "utils/console_logger.h"

int shmID;
void* shmAddr;

int* getRemainingTimeAddr(int id) {
    key_t shmKey = ftok(SHM_KEYFILE, id);
    if (shmKey == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    shmID = shmget(shmKey, sizeof(int), IPC_CREAT | 0666);
    if (shmID == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    shmAddr = shmat(shmID, NULL, 0);
    if (shmAddr == (void*)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    return (int*)shmAddr;
}

void detachRemainingTime(volatile int* remainingTime) {
    if (shmdt((void*)remainingTime) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }
}

int id;
void processClearResources(int) {
    printLog(CONSOLE_LOG_INFO, "Process", "Process %d terminating", id);

    if (shmdt(shmAddr) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }
    destroyClk(0);
    exit(0);
}

int main(int, char* argv[]) {
    id = atoi(argv[1]);
    signal(SIGINT, processClearResources);

    printLog(CONSOLE_LOG_INFO, "Process", "Process %d started with PID %d", id, getpid());
    syncClk();

    volatile int* remainingTime = getRemainingTimeAddr(id);

    // Busy-wait
    while (1) {
        if (*remainingTime <= 0) break;
    }

    int oldClk = getClk();
    detachRemainingTime(remainingTime);
    printLog(CONSOLE_LOG_SUCCESS, "Process", "Process %d finished at time %d", id, oldClk);
    raise(SIGINT);
    return 0;
}