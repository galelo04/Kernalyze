#ifndef NODE_H
#define NODE_H

struct Node {
    void *data;
    struct Node *next;
};

struct Node *createNode(void *data);

#endif
