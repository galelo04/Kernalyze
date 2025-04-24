#include "circularQueue.h"
#include <stdlib.h>

struct Queue* createQueue() {
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    q->rear = NULL;
    q->size = 0;
    return q;
}

void enqueue(struct Queue* queue, void* data) {
    if (queue == NULL) {
        return;
    }
    struct Node* newNode = createNode(data);

    if (queue->rear == NULL) {
        queue->rear = newNode;
        queue->rear->next = queue->rear;  // Point to itself
    } else {
        newNode->next = queue->rear->next;  // New node points to front
        queue->rear->next = newNode;        // Rear points to new node
        queue->rear = newNode;              // Update rear to new node
    }
    ++queue->size;
}

void* dequeue(struct Queue* queue) {
    if (queue == NULL || queue->rear == NULL) {
        return NULL;
    }

    struct Node* front = queue->rear->next;  // Get front node
    void* data = front->data;                // Store data to return
    if (front == queue->rear) {
        free(front);
        queue->rear = NULL;
    } else {
        queue->rear->next = front->next;  // Rear points to next node
        free(front);
    }

    --queue->size;
    if (queue->size == 0) {
        queue->rear = NULL;  // Reset rear if queue is empty
    }
    return data;  // Return dequeued data
}

void* peek(struct Queue* queue) {
    if (queue == NULL || queue->rear == NULL) {
        return NULL;
    }
    return queue->rear->next->data;  // Return front node data
}

void shiftQueue(struct Queue* queue, int n) {
    if (queue->rear == NULL || n <= 0) {
        return;  // Queue is empty or no shift needed
    }

    for (int i = 0; i < n; ++i) {
        queue->rear = queue->rear->next;
    }
}

void destroyQueue(struct Queue* queue) {
    if (queue == NULL) {
        return;
    }

    struct Node* current = queue->rear->next;  // Start from front
    struct Node* nextNode;

    do {
        nextNode = current->next;
        free(current);
        current = nextNode;
    } while (current != queue->rear->next);

    free(queue);
}