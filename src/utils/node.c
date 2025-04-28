#include "node.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

struct Node *createNode(void *data) {
    struct Node *newNode = (struct Node *)malloc(sizeof(struct Node));
    if (!newNode) {
        perror("malloc: ");
        exit(EXIT_FAILURE);
    }
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}