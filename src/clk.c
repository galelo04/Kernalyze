/*
 * This file is done for you.
 * Probably you will not need to change anything.
 * This file represents an emulated clock for simulation purpose only.
 * It is not a real part of operating system!
 */
#include "clk.h"
#include "utils/console_logger.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>

#define SHKEY 300
///==============================
// don't mess with this variable//
int *shmaddr = NULL;  //
//===============================

int shmid;

/* Clear the resources before exit */
void _cleanup(__attribute__((unused)) int signum) {
    shmctl(shmid, IPC_RMID, NULL);
    printInfo("clk", "Clock terminating!");
    exit(0);
}

void init_clk() {
    printInfo("clk", "Clock starting");
    signal(SIGINT, _cleanup);
    int clk = -1;
    // Create shared memory for one integer variable 4 bytes
    shmid = shmget(SHKEY, 4, IPC_CREAT | 0644);
    if ((long)shmid == -1) {
        perror("Error in creating shm!");
        exit(-1);
    }
    int *shmaddr = (int *)shmat(shmid, (void *)0, 0);
    if ((long)shmaddr == -1) {
        perror("Error in attaching the shm in clock!");
        exit(-1);
    }
    *shmaddr = clk; /* initialize shared memory */
}

void run_clk() {
    while (1) {
        usleep(500000);  // sleep for 0.5 seconds
        (*shmaddr)++;
    }
}

int get_clk() { return *shmaddr; }

void sync_clk() {
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1) {
        // Make sure that the clock exists
        printWarning("clk", "Wait! The clock not initialized yet!");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *)shmat(shmid, (void *)0, 0);
}

void destroy_clk(short terminateAll) {
    shmdt(shmaddr);
    if (terminateAll) {
        killpg(getpgrp(), SIGINT);
    }
}
