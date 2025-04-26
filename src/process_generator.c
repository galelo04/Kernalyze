#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "clk.h"
#include "scheduler.h"
#include "utils/console_logger.h"

void parseCommandLineArgs(int argc, char* argv[]);
int readProcessesFile(struct ProcessData** processes);
pid_t createClk();
pid_t createScheduler();
void checkChildProcess(int signum);
void runProcessGenerator(struct ProcessData* processes, int processCount, pid_t schedulerPID);
pid_t forkProcess(int id);
void sendProcesstoScheduler(struct ProcessData* pcb, int special);
void clearResources(int signum);

// Arguments
int argSchedulerType;
int argQuantum;
char* argFilename;

pid_t clkPID;
pid_t schedulerPID;
int pgMsgqid = -1;

int main(int argc, char* argv[]) {
    // Read scheduler, quantum, filename
    parseCommandLineArgs(argc, argv);

    // Message queue for communication with scheduler
    key_t key = ftok(MSG_QUEUE_KEYFILE, 1);
    if (key == -1) {
        perror("[PG] ftok");
        exit(1);
    }

    pgMsgqid = msgget(key, IPC_CREAT | 0666);
    if (pgMsgqid == -1) {
        perror("[PG] msgget");
        exit(1);
    }

    // Read processes from file
    struct ProcessData* processes = NULL;
    int processCount = readProcessesFile(&processes);

    if (processCount <= 0) {
        printf("No processes found in the file\n");
        exit(1);
    }

    // Clock & Scheduler creation
    clkPID = createClk();
    schedulerPID = createScheduler();

    // Signal handlers so when the scheduler dies
    signal(SIGINT, clearResources);
    signal(SIGCHLD, checkChildProcess);

    sync_clk();

    // Main loop
    runProcessGenerator(processes, processCount, schedulerPID);

    free(processes);
    destroy_clk(1);

    return 0;
}

void parseCommandLineArgs(int argc, char* argv[]) {
    argSchedulerType = -1;
    argQuantum = -1;
    argFilename = NULL;

    int i = 1;
    while (i < argc) {
        if (strcmp(argv[i], "-s") == 0) {  // Scheduler type
            if (i + 1 < argc) {
                if (strcmp(argv[i + 1], "rr") == 0) {
                    argSchedulerType = 0;
                } else if (strcmp(argv[i + 1], "srtn") == 0) {
                    argSchedulerType = 1;
                } else if (strcmp(argv[i + 1], "hpf") == 0) {
                    argSchedulerType = 2;
                } else {
                    printf("Invalid scheduler type\n");
                    exit(1);
                }
                i += 2;
            } else {
                printf("Invalid scheduler type\n");
                exit(1);
            }
        } else if (strcmp(argv[i], "-q") == 0) {  // Quantum for RR
            if (i + 1 < argc) {
                argQuantum = atoi(argv[i + 1]);
                if (argQuantum < 0) {
                    printf("Invalid quantum\n");
                    exit(1);
                }
                i += 2;
            } else {
                printf("Invalid quantum\n");
                exit(1);
            }
        } else if (strcmp(argv[i], "-f") == 0) {  // Filename
            if (i + 1 < argc) {
                argFilename = argv[i + 1];
                i += 2;
            } else {
                printf("Invalid filename\n");
                exit(1);
            }
        }
    }

    if (argFilename == NULL) {
        printf("Invalid filename\n");
        exit(1);
    }

    if (argSchedulerType == -1) {
        printf("Invalid scheduler type\n");
        exit(1);
    }

    if (argQuantum == -1 && argSchedulerType == 0) {
        printf("Invalid quantum for RR\n");
        exit(1);
    }
}

int readProcessesFile(struct ProcessData** processes) {
    FILE* file = fopen(argFilename, "r");
    if (file == NULL) {
        perror("Failed to open input file");
        return -1;
    }

    // Count the number of processes
    int count = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#') continue;  // comment line
        count++;
    }

    *processes = (struct ProcessData*)malloc(count * sizeof(struct ProcessData));

    // Read process data
    rewind(file);
    int i = 0;
    while (fgets(line, sizeof(line), file) && i < count) {
        if (line[0] == '#') continue;  // comment line

        int id, arrival, runtime, priority;
        if (sscanf(line, "%d %d %d %d", &id, &arrival, &runtime, &priority) == 4) {
            (*processes)[i].id = id;
            (*processes)[i].arriveTime = arrival;
            (*processes)[i].runningTime = runtime;
            (*processes)[i].priority = priority;
            i++;
        }
    }

    fclose(file);

    return count;
}

void clearResources(__attribute__((unused)) int signum) {
    if (msgctl(pgMsgqid, IPC_RMID, NULL) == -1) {
        perror("[PG] msgctl");
        exit(1);
    }

    printf("Message queue removed\n");
    exit(0);
}

pid_t createClk() {
    pid_t pid = fork();

    if (pid == -1) {
        perror("[PG] Failed to fork for clk");
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

pid_t createScheduler() {
    pid_t pid = fork();

    if (pid == -1) {
        perror("[PG] Failed to fork for scheduler");
        exit(1);
    }

    if (pid == 0) {
        initScheduler(argSchedulerType, argQuantum);
        runScheduler();
        exit(0);
    }

    return pid;
}

void checkChildProcess(__attribute__((unused)) int signum) {
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (pid == clkPID) {
            if (WIFEXITED(status)) {
                int exit_code = WEXITSTATUS(status);
                printInfo("PG", "Clock process terminated with exit code %d\n", exit_code);
            } else if (WIFSIGNALED(status)) {
                int signal_number = WTERMSIG(status);
                printInfo("PG", "Clock process terminated by signal %d\n", signal_number);
            }
        } else if (pid == schedulerPID) {
        }
    }
}

void runProcessGenerator(struct ProcessData* processes, int processCount, pid_t schedulerPID) {
    int prevClk = get_clk();

    int processIndex = 0;
    int noMoreProcesses = 0;

    while (1) {
        int currentClk = get_clk();
        if (currentClk == prevClk) continue;
        prevClk = currentClk;

        // Arrived processes
        while (processIndex < processCount && processes[processIndex].arriveTime <= currentClk) {
            processes[processIndex].pid = forkProcess(processes[processIndex].id);

            sendProcesstoScheduler(&processes[processIndex], 0);
            processIndex++;
        }

        // No more processes to send
        if (processIndex >= processCount && !noMoreProcesses) {
            sendProcesstoScheduler(NULL, 2);
            noMoreProcesses = 1;
        }

        // Tell scheduler that there are no more processes for this clock
        sendProcesstoScheduler(NULL, 1);

        // No more processes, wait for scheduler to finish
        if (noMoreProcesses && waitpid(schedulerPID, NULL, WNOHANG) > 0) break;
    }
}

pid_t forkProcess(int id) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("[PG] fork");
        exit(1);
    }

    if (pid == 0) {
        char idStr[32], schedulerPIDStr[32];

        // Convert id & runningTime to strings
        snprintf(idStr, sizeof(idStr), "%d", id);
        snprintf(schedulerPIDStr, sizeof(schedulerPIDStr), "%d", schedulerPID);

        char* args[4];
        args[0] = PROCESS_PATH;
        args[1] = idStr;
        args[2] = schedulerPIDStr;
        args[3] = NULL;

        // Stop the process until the scheduler starts it
        kill(getpid(), SIGSTOP);

        // Replace with process
        execv(PROCESS_PATH, args);

        // Error when execv
        perror("[PG] execv");
        exit(1);
    }

    return pid;
}

void sendProcesstoScheduler(struct ProcessData* p, int special) {
    struct PCBMessage msg;

    msg.mtype = MSG_TYPE_PCB;

    if (special == 1) {  // no more processes for this clk
        msg.pdata.id = -1;
    } else if (special == 2) {  // no more processes at all
        msg.pdata.id = -2;
    } else {  // normal process
        msg.pdata = *p;
    }

    if (msgsnd(pgMsgqid, &msg, sizeof(struct ProcessData), 0) == -1) {
        perror("[PG] msgsnd");
        exit(1);
    }
}