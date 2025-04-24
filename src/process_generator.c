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

void initProcessGenerator();

pid_t createClk();
pid_t createSheduler(int type);

int pgMsgqid = -1;

int main(int argc, char* argv[]) {
    for (int i = 0; i < argc; i++) {
        printf("arg %d: %s\n", i, argv[i]);
    }

    initProcessGenerator();
    int old_clk = 0;
    pid_t clk_pid = createClk();
    pid_t sheduler_pid = createSheduler(0);
    signal(SIGINT, clear_resources);
    sync_clk();
    while (1) {
        int current_clk = get_clk();
        if (current_clk == old_clk) continue;
        old_clk = current_clk;

        if (current_clk > 8) {
            break;
        }

        if (current_clk == 3) {
            struct PCB* pcb = newPCB(0, current_clk, 2, 1);
            if (pcb != NULL) {
                sendPCBtoScheduler(pcb);
                printf("Process %d is sent to the scheduler\n", pcb->id);
                free(pcb);
            } else {
                printf("No new process to send to the scheduler\n");
            }
        } else {
            sendPCBtoScheduler(NULL);
        }
    }

    destroy_clk(1);
}

void initProcessGenerator() {
    // Create message queue
    key_t key = ftok(MSG_QUEUE_KEYFILE, 1);
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    pgMsgqid = msgget(key, IPC_CREAT | 0666);
    if (pgMsgqid == -1) {
        perror("msgget");
        exit(1);
    }
}

void clear_resources(__attribute__((unused)) int signum) {
    // TODO Clears all resources in case of interruption
    if (msgctl(pgMsgqid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }
    printf("Message queue removed\n");
    exit(0);
}

void sendPCBtoScheduler(struct PCB* pcb) {
    struct PCBMessage msg;
    msg.mtype = MSG_TYPE_PCB;
    if (pcb == NULL) {
        msg.pcb.id = -1;
        msg.pcb.arriveTime = -1;
        msg.pcb.runningTime = -1;
        msg.pcb.priority = -1;
    } else
        msg.pcb = *pcb;

    if (msgsnd(pgMsgqid, &msg, sizeof(struct PCB), 0) == -1) {
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

pid_t createClk() {
    pid_t pid = fork();
    if (pid == -1) {
        perror("Failed to fork for clk");
        exit(1);
    }
    if (pid == 0) {
        init_clk();
        sync_clk();
        run_clk();
        exit(0);
    }
    return pid;
}

pid_t createSheduler(int type) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("Failed to fork for sheduler");
        exit(1);
    }
    if (pid == 0) {
        init_scheduler(type);
        run_scheduler();
        exit(0);
    }
    return pid;
}