/*
 * This file is done for you.
 * Probably you will not need to change anything.
 * This file represents an emulated clock for simulation purpose only.
 * It is not a real part of operating system!
 */
#include "clk.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>

#include "utils/console_logger.h"

#define SHKEY 300
///==============================
// don't mess with this variable//
int *clkShmAddr = NULL;  //
//===============================

int shmid;

/* Clear the resources before exit */
void _cleanup(__attribute__((unused)) int signum) {
    shmctl(shmid, IPC_RMID, NULL);
    printLog(CONSOLE_LOG_INFO, "CLK", "Clock terminating!");
    exit(0);
}

void initClk() {
    printLog(CONSOLE_LOG_INFO, "CLK", "Clock starting");
    signal(SIGINT, _cleanup);
    signal(SIGUSR2, SIG_IGN);
    int clk = -1;
    // Create shared memory for one integer variable 4 bytes
    shmid = shmget(SHKEY, 4, IPC_CREAT | 0644);
    if ((long)shmid == -1) {
        perror("Error in creating shm!");
        exit(-1);
    }
    int *clkShmAddr = (int *)shmat(shmid, (void *)0, 0);
    if ((long)clkShmAddr == -1) {
        perror("Error in attaching the shm in clock!");
        exit(-1);
    }
    *clkShmAddr = clk; /* initialize shared memory */
}

void runClk() {
    while (1) {
        usleep(100*1000);  // sleep for 1 seconds
        (*clkShmAddr)++;
        killpg(getpgrp(), SIGUSR2);
    }
}

int getClk() { return *clkShmAddr; }

void syncClk() {
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1) {
        // Make sure that the clock exists
        printLog(CONSOLE_LOG_WARNING, "CLK", "Wait! The clock not initialized yet!");
        usleep(100000);  // sleep for 0.1 seconds
        shmid = shmget(SHKEY, 4, 0444);
    }
    clkShmAddr = (int *)shmat(shmid, (void *)0, 0);
}

void destroyClk(short terminateAll) {
    shmdt(clkShmAddr);
    if (terminateAll) {
        killpg(getpgrp(), SIGINT);
    }
}
