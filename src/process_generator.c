#define _GNU_SOURCE
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
#include "utils/semaphore.h"

void parseCommandLineArgs(int argc, char* argv[]);
int readProcessesFile(struct ProcessData** processes);
pid_t createClk();
pid_t createScheduler();
void checkChildProcess(int signum);
void runProcessGenerator(struct ProcessData* processes, int processCount, pid_t schedulerPID);
pid_t forkProcess(int id);
void sendProcesstoScheduler(struct ProcessData* pcb, int special);
void pgClearResources(int signum);
void pgClkHandler(int);

// Arguments
int argSchedulerType;
int argQuantum;
char* argFilename;

pid_t clkPID = -1;
pid_t schedulerPID = -1;
int pgMsgqid = -1;
int pgCurrentClk = -1;
int pgSemid = -1;

volatile sig_atomic_t pgCleared = 0;

int main(int argc, char* argv[]) {
    // Initialize semaphore for clock
    pgSemid = initSemaphore(PG_SEMAPHORE);

    // Signal handlers so when the scheduler dies
    signal(SIGINT, pgClearResources);
    // signal(SIGCHLD, checkChildProcess);
    struct sigaction childSigaction;

    childSigaction.sa_handler = checkChildProcess;
    sigemptyset(&childSigaction.sa_mask);
    childSigaction.sa_flags = SA_NOCLDSTOP;

    if (sigaction(SIGCHLD, &childSigaction, 0) == -1) {
        perror("sigaction");
        raise(SIGINT);
    }

    // Signal handler for clock
    signal(SIGUSR2, pgClkHandler);

    // Read scheduler, quantum, filename
    parseCommandLineArgs(argc, argv);

    // Read processes from file
    struct ProcessData* processes = NULL;
    int processCount = readProcessesFile(&processes);

    // Message queue for communication with scheduler
    key_t key = ftok(MSG_QUEUE_KEYFILE, 1);
    if (key == -1) {
        perror("[PG] ftok");
        raise(SIGINT);
    }

    pgMsgqid = msgget(key, IPC_CREAT | 0666);
    if (pgMsgqid == -1) {
        perror("[PG] msgget");
        raise(SIGINT);
    }

    if (processCount <= 0) {
        printLog(CONSOLE_LOG_ERROR, "PG", "No processes found in the file");
        raise(SIGINT);
    }

    // Clock & Scheduler creation
    schedulerPID = createScheduler();
    clkPID = createClk();

    syncClk();

    // Main loop
    runProcessGenerator(processes, processCount, schedulerPID);

    free(processes);
    destroyClk(1);

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
                    printLog(CONSOLE_LOG_ERROR, "PG", "Invalid scheduler type");
                    raise(SIGINT);
                }
                i += 2;
            } else {
                printLog(CONSOLE_LOG_ERROR, "PG", "Invalid scheduler type");
                raise(SIGINT);
            }
        } else if (strcmp(argv[i], "-q") == 0) {  // Quantum for RR
            if (i + 1 < argc) {
                argQuantum = atoi(argv[i + 1]);
                if (argQuantum <= 0) {
                    printLog(CONSOLE_LOG_ERROR, "PG", "Invalid quantum");
                    raise(SIGINT);
                }
                i += 2;
            } else {
                printLog(CONSOLE_LOG_ERROR, "PG", "Invalid quantum");
                raise(SIGINT);
            }
        } else if (strcmp(argv[i], "-f") == 0) {  // Filename
            if (i + 1 < argc) {
                argFilename = argv[i + 1];
                i += 2;
            } else {
                printLog(CONSOLE_LOG_ERROR, "PG", "Invalid filename");
                raise(SIGINT);
            }
        } else {
            i++;
        }
    }

    if (argFilename == NULL) {
        printLog(CONSOLE_LOG_ERROR, "PG", "Invalid filename");
        raise(SIGINT);
    }

    if (argSchedulerType == -1) {
        printLog(CONSOLE_LOG_ERROR, "PG", "Invalid scheduler type");
        raise(SIGINT);
    }

    if (argQuantum == -1 && argSchedulerType == 0) {
        printLog(CONSOLE_LOG_ERROR, "PG", "Invalid quantum");
        raise(SIGINT);
    }
}

int readProcessesFile(struct ProcessData** processes) {
    FILE* file = fopen(argFilename, "r");
    if (file == NULL) {
        perror("[PG] Failed to open input file");
        raise(SIGINT);
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

        int id, arrival, runtime, priority, memsize;
        if (sscanf(line, "%d %d %d %d %d", &id, &arrival, &runtime, &priority, &memsize) == 5) {
            (*processes)[i].id = id;
            (*processes)[i].arriveTime = arrival;
            (*processes)[i].runningTime = runtime;
            (*processes)[i].priority = priority;
            (*processes)[i].memsize = memsize;
            i++;
        }
    }

    fclose(file);

    return count;
}

void pgClearResources(__attribute__((unused)) int signum) {
    if (pgCleared) return;
    pgCleared = 1;
    printLog(CONSOLE_LOG_INFO, "PG", "Process generator terminating");

    // Clean up message queue if it exists
    if (pgMsgqid != -1 && msgctl(pgMsgqid, IPC_RMID, NULL) == -1)
        perror("[PG] Failed to remove message queue");

    destroySemaphore(pgSemid);

    exit(0);
}

pid_t createClk() {
    pid_t pid = fork();

    if (pid == -1) {
        perror("[PG] Failed to fork for clk");
        raise(SIGINT);
    }

    if (pid == 0) {
        initClk();
        syncClk();
        runClk();
        exit(0);
    }

    return pid;
}

pid_t createScheduler() {
    pid_t pid = fork();

    if (pid == -1) {
        perror("[PG] Failed to fork for scheduler");
        raise(SIGINT);
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
                printLog(CONSOLE_LOG_INFO, "PG", "Clock process terminated with exit code %d",
                         exit_code);
            } else if (WIFSIGNALED(status)) {
                int signal_number = WTERMSIG(status);
                printLog(CONSOLE_LOG_INFO, "PG", "Clock process terminated by signal %d",
                         signal_number);
            }

            if (schedulerPID != -1) {
                printLog(CONSOLE_LOG_INFO, "PG", "Clock has stopped, stopping scheduler");
                kill(schedulerPID, SIGINT);
                waitpid(schedulerPID, NULL, 0);
                printLog(CONSOLE_LOG_INFO, "PG", "Scheduler has stopped");
                schedulerPID = -1;
            }

            pgClearResources(SIGINT);
        } else if (pid == schedulerPID) {
            if (WIFEXITED(status)) {
                int exit_code = WEXITSTATUS(status);
                printLog(CONSOLE_LOG_INFO, "PG", "Scheduler process terminated with exit code %d",
                         exit_code);
            } else if (WIFSIGNALED(status)) {
                int signal_number = WTERMSIG(status);
                printLog(CONSOLE_LOG_INFO, "PG", "Scheduler process terminated by signal %d",
                         signal_number);
            }

            if (clkPID != -1) {
                printLog(CONSOLE_LOG_INFO, "PG", "Scheduler has stopped, stopping clock");
                kill(clkPID, SIGINT);
                waitpid(clkPID, NULL, 0);
                printLog(CONSOLE_LOG_INFO, "PG", "Clock has stopped");
                clkPID = -1;
            }

            pgClearResources(SIGINT);
        } else {
            // send a message to the scheduler that the process has terminated
            struct PCBMessage msg;
            msg.mtype = MSG_TYPE_TERMINATION;
            msg.pdata.id = pid;
            msg.pdata.pid = pid;

            if (msgsnd(pgMsgqid, &msg, sizeof(struct ProcessData), 0) == -1) {
                perror("[PG] msgsnd");
                raise(SIGINT);
            }

            printLog(CONSOLE_LOG_INFO, "PG", "Process %d terminated", pid);
        }
    }
}

void runProcessGenerator(struct ProcessData* processes, int processCount, pid_t schedulerPID) {
    int processIndex = 0;
    int noMoreProcesses = 0;

    while (1) {
        down(pgSemid);
        printLog(CONSOLE_LOG_INFO, "PG", "CurrentClk: %d", pgCurrentClk);
        // Arrived processes
        while (processIndex < processCount && processes[processIndex].arriveTime == pgCurrentClk) {
            processes[processIndex].pid = forkProcess(processes[processIndex].id);

            sendProcesstoScheduler(&processes[processIndex], 0);
            processIndex++;
        }

        // No more processes to send
        if (processIndex >= processCount && !noMoreProcesses) {
            sendProcesstoScheduler(NULL, 2);
            noMoreProcesses = 1;
        }

        if (!noMoreProcesses) {
            // No more processes for this clock cycle
            sendProcesstoScheduler(NULL, 1);
        }

        // No more processes, wait for scheduler to finish
        if (noMoreProcesses && waitpid(schedulerPID, NULL, WNOHANG) > 0) break;
    }
    printLog(CONSOLE_LOG_INFO, "PG", "Process generator finished");
}

pid_t forkProcess(int id) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("[PG] fork");
        raise(SIGINT);
    }

    if (pid == 0) {
        char idStr[32];

        // Convert id to string
        snprintf(idStr, sizeof(idStr), "%d", id);

        char* args[3];
        args[0] = PROCESS_PATH;
        args[1] = idStr;
        args[2] = NULL;
        signal(SIGUSR2, SIG_IGN);
        // Stop the process until the scheduler starts it
        kill(getpid(), SIGSTOP);

        // Replace with process
        execv(PROCESS_PATH, args);

        // Error when execv
        perror("[PG] execv");
        raise(SIGINT);
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
        raise(SIGINT);
    }
}

void pgClkHandler(int) {
    pgCurrentClk = getClk();
    up(pgSemid);
}