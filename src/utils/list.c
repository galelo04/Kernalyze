#include "list.h"

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

struct List *createList() {
    struct List *list = (struct List *)malloc(sizeof(struct List));
    if (!list) {
        perror("malloc: ");
        exit(EXIT_FAILURE);
    }
    list->head = NULL;
    list->size = 0;
    return list;
}

void insertAtFront(struct List *list, void *data) {
    struct Node *newNode = createNode(data);
    newNode->next = list->head;
    list->head = newNode;
    list->size++;
}

void freeList(struct List *list) {
    struct Node *temp;
    while (list->head) {
        temp = list->head;
        list->head = list->head->next;
        free(temp);
    }
    list->size = 0;
    free(list);
}

/* cmp is a function pointer to a compare function it takes two void
pointers to the data needed to be compared
and return 0 if equal*/
void *findInList(struct List *list, void *data, int (*cmp)(void *, void *)) {
    struct Node *temp = list->head;
    while (temp) {
        if (cmp(temp->data, data) == 0) {
            return temp->data;
        }
        temp = temp->next;
    }
    return NULL;
}

void *removeFromList(struct List *list, void *data, int (*cmp)(void *, void *)) {
    struct Node *temp = list->head;
    struct Node *prev = NULL;
    while (temp) {
        if (cmp(temp->data, data) == 0) {
            if (prev) {
                prev->next = temp->next;
            } else {
                list->head = temp->next;
            }
            void *removedData = temp->data;
            free(temp);
            list->size--;
            return removedData;
        }
        prev = temp;
        temp = temp->next;
    }
    return NULL;
}