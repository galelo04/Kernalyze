#ifndef MINHEAP_C
#define MINHEAP_C
#include "vector.h"

typedef struct {
    void *data;
    int priority;
} heap_node;


typedef struct {
    Vector* vector;  // Vector to store heap_node pointers
    int size;        // Number of elements in the heap
} heap;



// Function to get the priority of a heap node
int get_node_priority(void* node_ptr) ;

// Define a createHeap function
heap* createHeap() ;

// Defining insertHelper function (bubble up)
void insertHelper(heap* h, int index) ;

// Heapify function (bubble down)
void heapify(heap* h, int index) ;

// Extract the minimum element from the heap
heap_node* extractMin(heap* h) ;

// Insert a new element into the heap
void insert(heap* h, void* data, int priority) ;

// Print the heap's priorities (for debugging)
void printHeap(heap* h) ;

// Function to free the heap and all its nodes
void destroyHeap(heap* h) ;


#endif