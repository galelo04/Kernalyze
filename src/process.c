#include "clk.h"
#include "defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#include <signal.h>

int main(int argc, char* argv[]) {
    for (int i = 0; i < argc; ++i) {
        printf("arg %d: %s\n", i, argv[i]);
    }

    sync_clk();

    int id = atoi(argv[1]);
    int runningTime = atoi(argv[2]);

    key_t shmKey = ftok(SHM_KEYFILE, id);
    if (shmKey == -1) {
        perror("ftok");
        return -1;
    }
    int shmID = shmget(shmKey, sizeof(int), IPC_CREAT | 0666);
    if (shmID == -1) {
        perror("shmget");
        return -1;
    }

    int* remainingTime = shmat(shmID, NULL, 0);
    if (remainingTime == (void*)-1) {
        perror("shmat");
        return -1;
    }
    *remainingTime = runningTime;
    printf("[Process] %d: remainingTime=%d\n", atoi(argv[1]), *remainingTime);
    // Simulate process execution
    int oldClk = get_clk();
    int currentTime;
    while (1) {
        if ((currentTime = get_clk()) != oldClk) {
            (*remainingTime)--;
            printf("[Process] %d: remainingTime=%d\n", atoi(argv[1]), *remainingTime);
            oldClk = currentTime;
        }
        if (*remainingTime <= 0) {
            break;
        }
    }

    printf("[Process] %d: finished\n", atoi(argv[1]));
    // Detach and remove shared memory
    if (shmdt(remainingTime) == -1) {
        perror("shmdt");
        return -1;
    }
    kill(getppid(), SIGUSR1);  // Notify the parent process (scheduler)

    return 0;
}
