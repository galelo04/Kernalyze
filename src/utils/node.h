#ifndef NODE_H
#define NODE_H

#include <stddef.h>

struct Node {
    void *data;
    struct Node *next;
};

struct Node *createNode(void *data) {
    struct Node *newNode = (struct Node *)malloc(sizeof(struct Node));
    if (!newNode) {
        perror("malloc: ");
        exit(1);
    }
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

#endif
