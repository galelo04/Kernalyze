#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>  // for pid_t
#include <unistd.h>     // for fork, execl

#include "clk.h"

void clear_resources(int);
int main(int argc, char* argv[]) {
    for (int i = 0; i < argc; i++) {
        printf("arg %d: %s\n", i, argv[i]);
    }

    pid_t clk_pid = fork();
    if (clk_pid == 0) {
        signal(SIGINT, clear_resources);
        sync_clk();
        while (1) {
            int x = get_clk();
            printf("current time is %d\n", x);
            sleep(1);
            if (x > 4) {
                break;
            }
        }
        // TODO:
        // - A process should spawn at its arrival time
        // - Spawn the scheduler for handling context switching
        destroy_clk(1);
    } else {
        init_clk();
        sync_clk();
        run_clk();
    }
}

void clear_resources(__attribute__((unused)) int signum) {
    // TODO Clears all resources in case of interruption
}
