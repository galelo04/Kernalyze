#include "semaphore.h"
#include "../defs.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>

union semun {
    int val;
};  // Union for semctl arguments

int init_semaphore(int id) {
    key_t key = ftok(SEM_KEYFILE, id);  // Generate a unique key for the semaphore
    if (key == -1) {
        perror("ftok failed");
        exit(1);
    }
    // Create a semaphore set with one semaphore
    int semid = semget(key, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget failed");
        exit(1);
    }

    // Initialize the semaphore to 0
    union semun arg = {0};  // Set the initial value to 0
    if (semctl(semid, 0, SETVAL, arg) == -1) {
        perror("semctl failed");
        exit(1);
    }
    return semid;  // Return the semaphore ID
}

void down(int semid) {
    struct sembuf op = {0, -1, 0};     // Operation: decrement semaphore 0 by 1
    if (semop(semid, &op, 1) == -1) {  // Perform the operation
        perror("down failed");
        exit(1);
    }
}

// Function to perform a semaphore "up" (increment) operation
void up(int semid) {
    struct sembuf op = {0, 1, 0};      // Operation: increment semaphore 0 by 1
    if (semop(semid, &op, 1) == -1) {  // Perform the operation
        perror("up failed");
        exit(1);
    }
}

void set_semaphore(int semid, int value) {
    union semun arg;
    arg.val = value;                            // Set the semaphore value
    if (semctl(semid, 0, SETVAL, arg) == -1) {  // Set the semaphore value
        perror("semctl failed");
        exit(1);
    }
}

void destroy_semaphore(int semid) {
    if (semctl(semid, 0, IPC_RMID) == -1) {  // Remove the semaphore
        perror("semctl failed");
        exit(1);
    }
}