#ifndef CIRCULARQUEUE_H
#define CIRCULARQUEUE_H
#include "node.h"

struct Queue {
    struct Node *front, *rear;
};


struct Queue* createQueue();
// Function to insert element in a Circular queue
void enQueueRear(struct Queue* queue,void * data) ;
void enQueueFront(struct Queue* queue,void * data) ;
void deQueueRear(struct Queue  *queue);
void deQueueFront(struct Queue  *queue);

void * peekRear(struct Queue * queue);
void * peekFront(struct Queue * queue);


#endif CIRCULARQUEUE_H