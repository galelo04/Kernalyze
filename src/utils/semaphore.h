#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

int init_semaphore(int id);

void up(int semid);

void down(int semid);

void set_semaphore(int semid, int value);

void destroy_semaphore(int semid);
#endif  // SEMAPHORE_H_