#ifndef NODE_H
#define NODE_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

struct Node {
    void *data;
    struct Node *next;
};

struct Node *createNode(void *data);

#endif
