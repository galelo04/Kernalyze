#ifndef MINHEAP_H
#define MINHEAP_H
#include "vector.h"

struct heap_node {
    void* data;
    int priority;
};

struct Heap {
    Vector* vector;  // Vector to store heap_node pointers
    int size;        // Number of elements in the Heap
};

/* example to use correctly

    // Create a new Heap
    Heap* h = createHeap();
    if (h == NULL) {
        return 1;
    }

    PCB a, b;

    // Insert some elements
    insert(h, &a, 10);
    insert(h, &b, 5);

    a.pid = 1;
    b.pid = 2;

    // Print the Heap
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

// Function to get the priority of a Heap node
int get_node_priority(void* node_ptr);

// Defining insertHelper function (bubble up)
void insertHelper(struct Heap* h, int index);

// Heapify function (bubble down)
void heapify(struct Heap* h, int index);

// Define a createHeap function
struct Heap* heap_create(void);

// Extract the minimum element from the Heap
int heap_extract_min(struct Heap* h, void** data, int* priority);

// Insert a new element into the Heap
void heap_insert(struct Heap* h, void* data, int priority);

// Check if the Heap is empty
int heap_is_empty(const struct Heap* h);

// Print the Heap's priorities (for debugging)
void heap_print(const struct Heap* h);

// Function to free the Heap and all its nodes
void heap_destroy(struct Heap* h);

#endif  // MINHEAP_H