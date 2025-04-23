#include "circularQueue.h"

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

struct Queue* createQueue() {
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
    return q;
}
// Function to insert element in a Circular queue
void enqueueRear(struct Queue* queue, void* data) {
    struct Node* newNode = createNode(data);

    if (queue->front == NULL)
        queue->front = newNode;
    else
        queue->rear->next = newNode;

    queue->rear = newNode;
    queue->rear->next = queue->front;
}
void enqueueFront(struct Queue* queue, void* data){

    struct Node* newNode = createNode(data);

    if (queue->rear == NULL)
        queue->rear = newNode;
    else
        queue->front->next = newNode;

    queue->front = newNode;
    queue->front->next = queue->rear;

}
void dequeueRear(struct Queue* queue){
    // if queue is empty
    if (queue->front == NULL) {
        return;
    }

    // If this is the last node to be deleted
    if (queue->front == queue->rear) {
        free(queue->front);
        queue->front = queue->rear = NULL;
    } else {
        struct Node* current = queue->front;
        
        // Find the node before rear
        while (current->next != queue->rear) {
            current = current->next;
        }
        
        // Remove the rear node
        free(queue->rear);
        queue->rear = current;
        queue->rear->next = queue->front;
    }
}

void dequeueFront(struct Queue* queue){

    // if queue is empty
    if (queue->front == NULL) {
        return ;
    }

    // If this is the last node to be deleted
    if (queue->front == queue->rear) {
        free(queue->front);
        queue->front = queue->rear = NULL;
    } else {
        struct Node* temp = queue->front;
        queue->front = queue->front->next;
        queue->rear->next = queue->front;
        free(temp);
    }

}

void* peekFront(struct Queue* queue){
    if(queue->front==NULL)return NULL;
    return queue->front->data;
}
void* peekRear(struct Queue* queue){
    if(queue->rear==NULL)return NULL;
    return queue->rear->data;

}