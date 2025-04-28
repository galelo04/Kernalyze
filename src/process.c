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
void* rtShmAddr;

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
    rtShmAddr = shmat(shmID, NULL, 0);
    if (rtShmAddr == (void*)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    return (int*)rtShmAddr;
}

int id;
void processClearResources(int) {
    printLog(CONSOLE_LOG_INFO, "Process", "Process %d terminating", id);

    if (rtShmAddr != NULL && rtShmAddr != (void*)-1) {
        if (shmdt(rtShmAddr) == -1) {
            perror("shmdt process");
        }
        rtShmAddr = NULL;
    }

    destroyClk(0);
    exit(0);
}

int main(int, char* argv[]) {
    id = atoi(argv[1]);
    signal(SIGINT, processClearResources);
    signal(SIGTERM, processClearResources);  // Add SIGTERM handler

    printLog(CONSOLE_LOG_INFO, "Process", "Process %d started with PID %d", id, getpid());
    syncClk();

    volatile int* remainingTime = getRemainingTimeAddr(id);

    // Process execution
    while (1)
        if (*remainingTime <= 0) break;

    // Process finished
    int oldClk = getClk();
    printLog(CONSOLE_LOG_SUCCESS, "Process", "Process %d finished at time %d", id, oldClk);
    raise(SIGINT);
    return 0;
}