#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>  // for pid_t
#include <unistd.h>     // for fork, execl
#include <sys/msg.h>    // for msgget, msgsnd
#include <sys/wait.h>

#include "clk.h"
#include "scheduler.h"
#include "utils/console_logger.h"

void clear_resources(int);
void checkChildProcess(int);

void sendProcesstoScheduler(struct ProcessData* pcb);
struct ProcessData* newProcessData(int id, int arriveTime, int runningTime, int priority);

void initProcessGenerator();

pid_t createClk();
pid_t createSheduler(int type, int quantum);
pid_t clk_pid;
pid_t sheduler_pid;

int pgMsgqid = -1;

int main(int argc, char* argv[]) {
    for (int i = 0; i < argc; i++) {
        printf("arg %d: %s\n", i, argv[i]);
    }

    initProcessGenerator();
    int oldClk = 0;
    clk_pid = createClk();
    sheduler_pid = createSheduler(0, 1);
    signal(SIGINT, clear_resources);
    signal(SIGCHLD, checkChildProcess);
    sync_clk();
    while (1) {
        int currentClk = get_clk();
        if (currentClk == oldClk) continue;
        oldClk = currentClk;

        if (currentClk > 15) {
            break;
        }

        if (currentClk == 3 || currentClk == 4) {
            struct ProcessData* pdata = newProcessData(currentClk, currentClk, 2, 1);
            if (pdata != NULL) {
                sendProcesstoScheduler(pdata);
                free(pdata);
            }
        } else {
            sendProcesstoScheduler(NULL);
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

void sendProcesstoScheduler(struct ProcessData* pdata) {
    struct PCBMessage msg;
    msg.mtype = MSG_TYPE_PCB;
    if (pdata == NULL) {
        msg.pdata.id = -1;
        msg.pdata.arriveTime = -1;
        msg.pdata.runningTime = -1;
        msg.pdata.priority = -1;
    } else
        msg.pdata = *pdata;

    if (msgsnd(pgMsgqid, &msg, sizeof(struct ProcessData), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }
}

struct ProcessData* newProcessData(int id, int arriveTime, int runningTime, int priority) {
    struct ProcessData* pdata = (struct ProcessData*)malloc(sizeof(struct ProcessData));
    if (pdata == NULL) {
        perror("Failed to allocate memory for ProcessData");
        return NULL;
    }
    pdata->id = id;
    pdata->arriveTime = arriveTime;
    pdata->runningTime = runningTime;
    pdata->priority = priority;
    return pdata;
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

pid_t createSheduler(int type, int quantum) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("Failed to fork for sheduler");
        exit(1);
    }
    if (pid == 0) {
        init_scheduler(type, quantum);
        run_scheduler();
        exit(0);
    }
    return pid;
}

void checkChildProcess(__attribute__((unused))int signum) {
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (pid == clk_pid) {
            if (WIFEXITED(status)) {
                int exit_code = WEXITSTATUS(status);
                printInfo("PG", "Clock process terminated with exit code %d\n", exit_code);
            } else if (WIFSIGNALED(status)) {
                int signal_number = WTERMSIG(status);
                printInfo("PG", "Clock process terminated by signal %d\n", signal_number);
            }
        } else if (pid == sheduler_pid) {
            if (WIFEXITED(status)) {
                int exit_code = WEXITSTATUS(status);
                printInfo("PG", "Scheduler process terminated with exit code %d\n", exit_code);
            } else if (WIFSIGNALED(status)) {
                int signal_number = WTERMSIG(status);
                printInfo("PG", "Scheduler process terminated by signal %d\n", signal_number);
            }
        }
    }
}