#include "process.h"

// Simulate CPU-bound work with periodic checks for preemption or completion
void simulate_cpu_bound_work(int total_runtime, int quantum, int pipe_read_fd, int pipe_write_fd) {
    printf("Process %d: Starting CPU-bound work for %d seconds with quantum %d...\n", getpid(), total_runtime, quantum);

    int execution_time = 0; // Track how long the process has been running
    char buffer[256];       // Buffer for reading from the pipe

    while (execution_time < total_runtime) {
        int time_to_run = (total_runtime - execution_time > quantum) ? quantum : (total_runtime - execution_time);

        printf("Process %d: Running for %d seconds (remaining runtime: %d seconds)...\n", getpid(), time_to_run, total_runtime - execution_time);

        // Simulate work for the current quantum
        for (int i = 0; i < time_to_run; i++) {
            sleep(1); // Simulate 1 second of work
            execution_time++;

            // Get the current time from the shared clock
            int current_time = get_clk();
            printf("Process %d: Executed for %d/%d seconds at time %d.\n", getpid(), execution_time, total_runtime, current_time);

            // Check if the scheduler sent a preemption signal
            ssize_t bytes_read = read(pipe_read_fd, buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0'; // Null-terminate the string
                if (strcmp(buffer, "STOP") == 0) {
                    printf("Process %d: Received STOP signal from scheduler at time %d.\n", getpid(), current_time);
                    return;
                }
            }

            if (execution_time >= total_runtime) {
                break;
            }
        }

        if (execution_time < total_runtime) {
            notify_scheduler(pipe_write_fd, "QUANTUM_END");
        }
    }

    printf("Process %d: Finished CPU-bound work.\n", getpid());

    notify_scheduler(pipe_write_fd, "FINISHED");
}

void notify_scheduler(int write_fd, const char *message) {
    int current_time = get_clk(); // Get the current time from the shared clock
    printf("Process %d: Notifying scheduler with message '%s' at time %d...\n", getpid(), message, current_time);

    // Write the message to the pipe
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

    int runtime = atoi(argv[1]);         // Total runtime in seconds
    int quantum = atoi(argv[2]);         // Quantum size in seconds
    int pipe_read_fd = atoi(argv[3]);   // Read file descriptor for IPC communication
    int pipe_write_fd = atoi(argv[4]);  // Write file descriptor for IPC communication

    sync_clk();

    // Simulate CPU-bound work
    simulate_cpu_bound_work(runtime, quantum, pipe_read_fd, pipe_write_fd);

    // Exit successfully (scheduler does NOT terminate this process)
    printf("Process %d: Exiting.\n", getpid());
    return EXIT_SUCCESS;
}