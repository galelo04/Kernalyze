#ifndef CIRCULARQUEUE_H
#define CIRCULARQUEUE_H
#include "node.h"

struct Queue {
    struct Node* rear;
    int size;
};

struct Queue* createQueue();
// Function to insert element in a Circular queue
void enqueue(struct Queue* queue, void* data);
void* dequeue(struct Queue* queue);
void* peek(struct Queue* queue);
void shiftQueue(struct Queue* queue, int n);
void freeQueue(struct Queue* queue);

#endif  // CIRCULARQUEUE_H