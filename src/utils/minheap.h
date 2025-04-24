#ifndef MINHEAP_H
#define MINHEAP_H
#include "vector.h"

typedef struct {
    void* data;
    int priority;
} heap_node;

typedef struct {
    Vector* vector;  // Vector to store heap_node pointers
    int size;        // Number of elements in the heap
} heap;

/* example to use correctly

    // Create a new heap
    heap* h = createHeap();
    if (h == NULL) {
        return 1;
    }

    PCB a, b;

    // Insert some elements
    insert(h, &a, 10);
    insert(h, &b, 5);

    a.pid = 1;
    b.pid = 2;

    // Print the heap
    printHeap(h);

    // Extract the minimum element
    void* data;
    int priority;
    int ok = heap_extract_min(h, &data, &priority);
    if (ok) {
        PCB* p = (PCB*)data;
        printf("Extracted min: pid = %d, priority = %d\n", p->pid, priority);
    } else {
        printf("Heap is empty.\n");
    }

    // Clean up
    destroyHeap(h);
*/

// Function to get the priority of a heap node
int get_node_priority(void* node_ptr);

// Defining insertHelper function (bubble up)
void insertHelper(heap* h, int index);

// Heapify function (bubble down)
void heapify(heap* h, int index);

// Define a createHeap function
heap* heap_create(void);

// Extract the minimum element from the heap
int heap_extract_min(heap* h, void** data, int* priority);

// Insert a new element into the heap
void heap_insert(heap* h, void* data, int priority);

// Print the heap's priorities (for debugging)
void heap_print(const heap* h);

// Function to free the heap and all its nodes
void heap_destroy(heap* h);

#endif