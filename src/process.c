#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#include "clk.h"
#include "defs.h"
#include "utils/console_logger.h"

int* getRemainingTimeAddr(int id) {
    key_t shmKey = ftok(SHM_KEYFILE, id);
    if (shmKey == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    int shmID = shmget(shmKey, sizeof(int), IPC_CREAT | 0666);
    if (shmID == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    void* shmAddr = shmat(shmID, NULL, 0);
    if (shmAddr == (void*)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    return (int*)shmAddr;
}

void detachRemainingTime(int* remainingTime) {
    if (shmdt(remainingTime) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }
}

int main(int, char* argv[]) {
    int id = atoi(argv[1]);

    printInfo("Process", "Process %d started with PID %d", id, getpid());
    syncClk();

    int* remainingTime = getRemainingTimeAddr(id);
    int oldClk = getClk();

    // Busy-wait
    while (1) {
        int clk = getClk();
        if (clk != oldClk) {
            oldClk = clk;
            if (*remainingTime <= 0) break;
        }
    }

    detachRemainingTime(remainingTime);
    printSuccess("Process", "Process %d finished at time %d", id, oldClk);

    return 0;
}