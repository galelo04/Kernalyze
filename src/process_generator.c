#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>  // for pid_t
#include <unistd.h>     // for fork, execl
#include <sys/msg.h>    // for msgget, msgsnd

#include "clk.h"
#include "scheduler.h"

void clear_resources(int);
void sendPCBtoScheduler(struct PCB* pcb);
struct PCB* newPCB(int id, int arriveTime, int runningTime, int priority);

int main(int argc, char* argv[]) {
    for (int i = 0; i < argc; i++) {
        printf("arg %d: %s\n", i, argv[i]);
    }

    int old_clk = 0;

    pid_t clk_pid = fork();
    if (clk_pid == 0) {
        init_clk();
        sync_clk();
        run_clk();
    } else {
        pid_t sheduler_pid = fork();
        if (sheduler_pid == 0) {
            init_scheduler();
            run_scheduler();
            exit(0);
        } else {
            signal(SIGINT, clear_resources);
            sync_clk();
            while (1) {
                int current_clk = get_clk();
                if (current_clk != old_clk) {
                    printf("current time is %d\n", current_clk);
                    if (current_clk == 3) {
                        struct PCB* pcb = newPCB(0, 1, 2, 3);
                        if (pcb != NULL) {
                            sendPCBtoScheduler(pcb);
                            kill(sheduler_pid, SIGUSR2);
                            printf("Process %d is sent to the scheduler\n", pcb->id);
                        } else {
                            printf("No new process to send to the scheduler\n");
                        }
                    }
                    if (current_clk > 8) {
                        break;
                    }
                }
                old_clk = current_clk;
            }
            // TODO:
            // - A process should spawn at its arrival time
            // - Spawn the scheduler for handling context switching
            destroy_clk(1);
        }
    }
}

void clear_resources(__attribute__((unused)) int signum) {
    // TODO Clears all resources in case of interruption
    exit(0);
}

void sendPCBtoScheduler(struct PCB* pcb) {
    key_t key = ftok(MSG_QUEUE_KEYFILE, 1);
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    int msgqid = msgget(key, IPC_CREAT | 0666);
    if (msgqid == -1) {
        perror("msgget");
        exit(1);
    }

    struct PCBMessage msg;
    msg.mtype = MSG_TYPE_PCB;
    msg.pcb = *pcb;

    if (msgsnd(msgqid, &msg, sizeof(struct PCB), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }
}

struct PCB* newPCB(int id, int arriveTime, int runningTime, int priority) {
    struct PCB* pcb = (struct PCB*)malloc(sizeof(struct PCB));
    if (pcb == NULL) {
        perror("Failed to allocate memory for PCB");
        return NULL;
    }
    pcb->id = id;
    pcb->arriveTime = arriveTime;
    pcb->runningTime = runningTime;
    pcb->priority = priority;
    return pcb;
}