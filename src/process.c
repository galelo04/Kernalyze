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

void detachRemainingTime(int* remainingTime) {
    if (shmdt((void*)remainingTime) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }
}

int id;
void processClearResources(int) {
    printInfo("Process", "Process %d terminating", id);

    if (shmdt(shmAddr) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }

    if (shmctl(shmID, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(EXIT_FAILURE);
    }
    exit(0);
}

int main(int, char* argv[]) {
    id = atoi(argv[1]);
    signal(SIGINT, processClearResources);

    printInfo("Process", "Process %d started with PID %d", id, getpid());
    syncClk();

    int* remainingTime = getRemainingTimeAddr(id);

    // Busy-wait
    while (1) {
        if (*remainingTime <= 0) break;
    }

    int oldClk = getClk();
    detachRemainingTime(remainingTime);
    printSuccess("Process", "Process %d finished at time %d", id, oldClk);

    return 0;
}