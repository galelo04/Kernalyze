#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "clk.h"

void run_process(int runtime) {
    if (runtime <= 0) {
        fprintf(stderr, "Process %d: Invalid runtime: %d\n", getpid(), runtime);
        return;
    }

    sync_clk();

    printf("Process %d: Starting CPU-bound work for %d seconds...\n", getpid(), runtime);

    int remaining_time = runtime;
    int last_clock = get_clk();

    while (remaining_time > 0) {
        // Wait for the next clock tick
        while (get_clk() == last_clock) {
            usleep(1000);
        }

        last_clock = get_clk();
        remaining_time--;

        printf("Process %d: Executed for 1 second, %d seconds remaining at time %d.\n", getpid(),
               remaining_time, last_clock);
    }

    printf("Process %d: Finished CPU-bound work.\n", getpid());

    pid_t parent_pgid = getpgid(getppid());
    if (parent_pgid == -1) {
        perror("Process: Failed to get parent process group ID");
    } else {
        if (kill(-parent_pgid, SIGUSR1) == -1) {
            perror("Process: Failed to send SIGUSR1 to parent process group");
        } else {
            printf("Process %d: Sent SIGUSR1 to parent process group %d at time %d.\n", getpid(),
                   parent_pgid, get_clk());
        }
    }

    destroy_clk(0);
}