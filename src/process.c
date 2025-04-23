#include "clk.h"

/* Modify this file as needed*/

void run_process(int runtime) {
    sync_clk();

    // placeholder
    while (runtime) {
        runtime--;
    }
    // TODO: Keep running the process till its runtime is over

    destroy_clk(0);
}
