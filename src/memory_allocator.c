#include "memory_allocator.h"

#include "defs.h"
#include "utils/console_logger.h"

int nextPowerOfTwo(int N) {
    if (!(N & (N - 1))) return N;
    return (1U << (sizeof(N) * 8 - 1)) >> (__builtin_clz(N) - 1);
}

struct MemoryBlock* mainMemory = NULL;
FILE* memoryLogFile = NULL;

void initMemory() {
    mainMemory = createEmptyBlock(0, TOTAL_MEMORY - 1, NULL);
    memoryLogFile = fopen(MEMORY_LOG_FILE, "w");
    if (memoryLogFile == NULL) {
        perror("[Memory] Failed to open memory log file");
        exit(EXIT_FAILURE);
    }
    fflush(memoryLogFile);
}

int allocateMemory(pid_t pid, int size) {
    int allocationSize = nextPowerOfTwo(size);
    return allocateHelper(mainMemory, pid, size, allocationSize);
}

int allocateHelper(struct MemoryBlock* root, pid_t pid, int size, int allocationSize) {
    if (root == NULL) return -1;

    int isLeaf = root->left == NULL && root->right == NULL;

    if (isLeaf) {
        if (root->isFree && root->allocationSize == allocationSize) {
            root->pid = pid;
            root->size = size;
            root->isFree = 0;
            memoryLogger(root, ALLOCATE, memoryLogFile);
            return 0;  // Success
        }

        if (root->allocationSize < allocationSize || !root->isFree) return -1;

        // Else: split into smaller nodes
        int mid = root->start + (root->end - root->start) / 2;
        root->left = createEmptyBlock(root->start, mid, root);
        root->right = createEmptyBlock(mid + 1, root->end, root);
        root->isFree = 0;
        return allocateHelper(root->left, pid, size, allocationSize);
    }

    int leftResult = allocateHelper(root->left, pid, size, allocationSize);
    if (leftResult == 0) return 0;
    return allocateHelper(root->right, pid, size, allocationSize);
}

struct MemoryBlock* createEmptyBlock(int start, int end, struct MemoryBlock* parent) {
    struct MemoryBlock* root = (struct MemoryBlock*)malloc(sizeof(struct MemoryBlock));
    root->pid = -1;
    root->size = 0;
    root->isFree = 1;
    root->allocationSize = end - start + 1;
    root->start = start;
    root->end = end;
    root->left = NULL;
    root->right = NULL;
    root->parent = parent;
    return root;
}

int freeMemory(pid_t pid) {
    struct MemoryBlock* root = findBlockByPid(mainMemory, pid);
    if (root == NULL) return -1;
    memoryLogger(root, FREE, memoryLogFile);

    root->isFree = 1;
    root->pid = -1;  // Or 0, depending on your logic

    freeMemoryHelper(root);

    return 0;
}

void freeMemoryHelper(struct MemoryBlock* root) {
    if (root == NULL || root->parent == NULL) return;
    struct MemoryBlock* parent = root->parent;
    struct MemoryBlock* sibling = (parent->left == root) ? parent->right : parent->left;
    if (sibling && sibling->isFree && sibling->left == NULL && sibling->right == NULL) {
        parent->left = NULL;
        parent->right = NULL;
        parent->isFree = 1;
        parent->pid = -1;
        free(root);
        free(sibling);
        freeMemoryHelper(parent);
    }
}

struct MemoryBlock* findBlockByPid(struct MemoryBlock* root, pid_t pid) {
    if (root == NULL) return NULL;
    if (root->pid == pid && !root->isFree) return root;
    struct MemoryBlock* child = findBlockByPid(root->left, pid);
    if (child != NULL) return child;
    child = findBlockByPid(root->right, pid);
    return child;
}

void memoryLogger(struct MemoryBlock* block, enum MEM_ACTION memAction, FILE* file) {
    if (memAction == ALLOCATE) {
        printLog(CONSOLE_LOG_INFO, "Memory",
                 "At time %d allocated %d bytes for process %d from %d to %d", getClk(),
                 block->size, block->pid, block->start, block->end);

        fprintf(file, "At time %d allocated %d bytes for process %d from %d to %d\n", getClk(),
                block->size, block->pid, block->start, block->end);
    } else if (memAction == FREE) {
        fprintf(file, "At time %d freed %d bytes from process %d from %d to %d\n", getClk(),
                block->size, block->pid, block->start, block->end);
    }
    fflush(file);
}

void destroyHelper(struct MemoryBlock* root) {
    if (root == NULL) return;
    destroyHelper(root->left);
    destroyHelper(root->right);
    free(root);
}

void destroyMemory() {
    destroyHelper(mainMemory);
    mainMemory = NULL;

    if (memoryLogFile != NULL) {
        fclose(memoryLogFile);
        memoryLogFile = NULL;
    }
}