#include "semaphore.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>

#include "../defs.h"

union semun {
    int val;
};  // Union for semctl arguments

int initSemaphore(int id) {
    key_t key = ftok(SEM_KEYFILE, id);  // Generate a unique key for the semaphore
    if (key == -1) {
        perror("ftok failed");
        raise(SIGINT);
    }
    // Create a semaphore set with one semaphore
    int semid = semget(key, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget failed");
        raise(SIGINT);
    }

    // Initialize the semaphore to 0
    union semun arg = {0};  // Set the initial value to 0
    if (semctl(semid, 0, SETVAL, arg) == -1) {
        perror("semctl failed");
        raise(SIGINT);
    }
    return semid;  // Return the semaphore ID
}

int getSemaphore(int id) {
    key_t key = ftok(SEM_KEYFILE, id);  // Generate a unique key for the semaphore
    if (key == -1) {
        perror("ftok failed");
        raise(SIGINT);
    }
    // Get the semaphore set with one semaphore
    int semid = semget(key, 1, 0666);
    if (semid == -1) {
        perror("semget failed");
        raise(SIGINT);
    }
    return semid;  // Return the semaphore ID
}

void down(int semid) {
    struct sembuf op = {0, -1, 0};  // Operation: decrement semaphore 0 by 1
    while (1) {
        if (semop(semid, &op, 1) == -1) {
            if (errno == EINTR) continue;  // retry if interrupted
            perror("semop down");
            raise(SIGINT);
        }
        break;
    }
}

// Function to perform a semaphore "up" (increment) operation
void up(int semid) {
    struct sembuf op = {0, 1, 0};      // Operation: increment semaphore 0 by 1
    if (semop(semid, &op, 1) == -1) {  // Perform the operation
        perror("up failed");
        raise(SIGINT);
    }
}

void set_semaphore(int semid, int value) {
    union semun arg;
    arg.val = value;                            // Set the semaphore value
    if (semctl(semid, 0, SETVAL, arg) == -1) {  // Set the semaphore value
        perror("semctl failed");
        raise(SIGINT);
    }
}

void destroySemaphore(int semid) {
    if (semctl(semid, 0, IPC_RMID) == -1) {  // Remove the semaphore
        perror("semctl failed");
    }
}