#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "clk.h"
#include "process.h"

// Simulate CPU-bound work with periodic checks for preemption or completion
void simulate_cpu_bound_work(int total_runtime, int quantum, int pipe_read_fd, int pipe_write_fd) {
    printf("Process %d: Starting CPU-bound work for %d seconds with quantum %d...\n", getpid(), total_runtime, quantum);

    int remaining_time = total_runtime; // Track remaining runtime
    char buffer[256];       // Buffer for reading from the pipe
    int last_clock = get_clk(); // Track last clock tick

    while (remaining_time > 0) {
        int time_to_run = (remaining_time > quantum) ? quantum : remaining_time;

        printf("Process %d: Running for %d seconds (remaining runtime: %d seconds)...\n", 
               getpid(), time_to_run, remaining_time);

        // Simulate work for the current quantum
        for (int i = 0; i < time_to_run; i++) {
            // Wait for next clock tick
            while (get_clk() == last_clock) {
                usleep(1000); // Brief sleep to prevent busy waiting
            }
            
            // Update clock and decrement remaining time
            last_clock = get_clk();
            remaining_time--;

            printf("Process %d: Executed for 1 second, %d seconds remaining at time %d.\n", 
                   getpid(), remaining_time, last_clock);

            // Check if the scheduler sent a preemption signal
            ssize_t bytes_read = read(pipe_read_fd, buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                if (strcmp(buffer, "STOP") == 0) {
                    printf("Process %d: Received STOP signal from scheduler at time %d.\n", 
                           getpid(), last_clock);
                    return;
                }
            }

            if (remaining_time <= 0) {
                break;
            }
        }

        if (remaining_time > 0) {
            notify_scheduler(pipe_write_fd, "QUANTUM_END");
        }
    }

    printf("Process %d: Finished CPU-bound work.\n", getpid());

    // Send SIGUSR1 to parent (scheduler)
    if (kill(getppid(), SIGUSR1) == -1) {
        perror("Process: Failed to send SIGUSR1 to scheduler");
    } else {
        printf("Process %d: Sent SIGUSR1 to scheduler at time %d.\n", getpid(), get_clk());
    }

    notify_scheduler(pipe_write_fd, "FINISHED");
}

void notify_scheduler(int write_fd, const char *message) {
    int current_time = get_clk();
    printf("Process %d: Notifying scheduler with message '%s' at time %d...\n", 
           getpid(), message, current_time);

    if (write(write_fd, message, strlen(message) + 1) == -1) {
        perror("Process: Failed to notify scheduler");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <runtime> <quantum> <pipe_read_fd> <pipe_write_fd>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int runtime = atoi(argv[1]);
    int quantum = atoi(argv[2]);
    int pipe_read_fd = atoi(argv[3]);
    int pipe_write_fd = atoi(argv[4]);

    sync_clk();

    simulate_cpu_bound_work(runtime, quantum, pipe_read_fd, pipe_write_fd);

    printf("Process %d: Exiting.\n", getpid());
    return EXIT_SUCCESS;
}