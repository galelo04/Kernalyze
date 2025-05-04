#ifndef MEMORY_ALLOCATOR_H
#define MEMORY_ALLOCATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "clk.h"

// Memory block structure for buddy allocation system
struct MemoryBlock {
    pid_t pid;
    int size;
    int allocationSize;
    int start;
    int end;
    int isFree;
    struct MemoryBlock* left;
    struct MemoryBlock* right;
    struct MemoryBlock* parent;
};

// Memory action enum
enum MEM_ACTION { ALLOCATE, FREE };

// Initialize the memory allocator
void initMemory();

// Allocate memory for a process
int allocateMemory(pid_t pid, int size);

int canAllocate(int size);

// Free memory from a process
int freeMemory(pid_t pid);

// Destroy the memory allocator
void destroyMemory();

// Helper functions
int nextPowerOfTwo(int N);
struct MemoryBlock* createEmptyBlock(int start, int end, struct MemoryBlock* parent);
struct MemoryBlock* findBlockByPid(struct MemoryBlock* root, pid_t pid);
int allocateHelper(struct MemoryBlock* root, pid_t pid, int size, int allocationSize);
void freeMemoryHelper(struct MemoryBlock* root);
void memoryLogger(struct MemoryBlock* block, enum MEM_ACTION memAction, FILE* file);
void destroyHelper(struct MemoryBlock* root);

#endif  // MEMORY_ALLOCATOR_H