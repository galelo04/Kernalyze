#ifndef PROCESS_H
#define PROCESS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

// Function prototypes
void simulate_cpu_bound_work(int total_runtime, int quantum, int pipe_read_fd, int pipe_write_fd);
void notify_scheduler(int write_fd, const char *message);

#endif // PROCESS_H