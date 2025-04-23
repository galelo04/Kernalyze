#include "process.h"

// Simulate CPU-bound work with periodic checks for preemption or completion
void simulate_cpu_bound_work(int total_runtime, int quantum, int pipe_read_fd, int pipe_write_fd) {
    printf("Process %d: Starting CPU-bound work for %d seconds with quantum %d...\n", getpid(), total_runtime, quantum);

    int execution_time = 0; // Track how long the process has been running
    char buffer[256];       // Buffer for reading from the pipe

    while (execution_time < total_runtime) {
        int time_to_run = (total_runtime - execution_time > quantum) ? quantum : (total_runtime - execution_time);

        printf("Process %d: Running for %d seconds (remaining runtime: %d seconds)...\n", getpid(), time_to_run, total_runtime - execution_time);

        for (int i = 0; i < time_to_run; i++) {
            sleep(1);
            execution_time++;

            ssize_t bytes_read = read(pipe_read_fd, buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                if (strcmp(buffer, "STOP") == 0) {
                    printf("Process %d: Received STOP signal from scheduler.\n", getpid());
                    return;
                }
            }

            printf("Process %d: Executed for %d/%d seconds.\n", getpid(), execution_time, total_runtime);

            if (execution_time >= total_runtime)
                break;
        }

        if (execution_time < total_runtime)
            notify_scheduler(pipe_write_fd, "QUANTUM_END");
    }

    printf("Process %d: Finished CPU-bound work.\n", getpid());
    notify_scheduler(pipe_write_fd, "FINISHED");
}

void notify_scheduler(int write_fd, const char *message) {
    printf("Process %d: Notifying scheduler with message '%s'...\n", getpid(), message);

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
    int pipe_read_fd = atoi(argv[3]);   // Read file descriptor for IPC communication
    int pipe_write_fd = atoi(argv[4]);  // Write file descriptor for IPC communication

    simulate_cpu_bound_work(runtime, quantum, pipe_read_fd, pipe_write_fd);

    printf("Process %d: Exiting.\n", getpid());
    return EXIT_SUCCESS;
}