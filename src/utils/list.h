#ifndef LIST_H
#define LIST_H
#include "node.h"

struct List {
    struct Node *head;
    int size;
};

struct List *createList();

void insertAtFront(struct List *list, void *data);

void freeList(struct List *list);

void *findInList(struct List *list, void *data, int (*cmp)(void *, void *));

void *removeFromList(struct List *list, void *data, int (*cmp)(void *, void *));

#endif
