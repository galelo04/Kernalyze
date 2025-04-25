#ifndef MEMORY_H
#define MEMORY_H
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include "clk.h"

#define TOTAL_MEMORY 1024

enum MEM_ACTION { ALLOCATE,
                  FREE };
struct MemoryBlock {
    pid_t pid;
    int size;

    int isFree;

    int allocationSize;
    int start;
    int end;

    struct MemoryBlock* left;
    struct MemoryBlock* right;
    struct MemoryBlock* parent;
};

// Helpers
int nextPowerOfTwo(int size);
struct MemoryBlock* createEmptyBlock(int start, int end, struct MemoryBlock* parent);
struct MemoryBlock* findBlockByPid(struct MemoryBlock* root, pid_t pid);
int allocateHelper(struct MemoryBlock* root, pid_t pid, int size, int allocationSize);
void destroyHelper();
void freeMemoryHelper(struct MemoryBlock* root);

// Initilize memory
void initMemory();
void destroyMemory();

int allocateMemory(pid_t pid, int size);
int freeMemory(pid_t pid);

// for logging:
void memoryLogger(struct MemoryBlock* block, enum MEM_ACTION memAction, FILE* file);

#endif  // MEMORY_H