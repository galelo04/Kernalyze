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

int main(__attribute__((unused)) int argc, char* argv[]) {
    printf("Scheduler PID: %d\n", atoi(argv[2]));
    sync_clk();

    int id = atoi(argv[1]);
    // int runningTime = atoi(argv[2]);

    int* remainingTime = getRemainingTimeAddr(id);

    char* processName = malloc(32);
    snprintf(processName, 32, "Process %d", id);

    int oldClk = get_clk();
    // printInfo(processName, "Started with running time %d at %d", runningTime, oldClk);
    while (*remainingTime > 1) {
        if (get_clk() != oldClk) {
            oldClk = get_clk();
        }
    }

    // printInfo(processName, " Finished at time %d", get_clk());

    // Detach shared memory
    detachRemainingTime(remainingTime);
    kill(atoi(argv[2]), SIGUSR1);  // Notify scheduler that process has finished
    return 0;
}
