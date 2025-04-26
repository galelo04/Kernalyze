#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

int initSemaphore(int id);

void up(int semid);

void down(int semid);

void set_semaphore(int semid, int value);

void destroySemaphore(int semid);
#endif  // SEMAPHORE_H_