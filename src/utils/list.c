#include "list.h"

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>



struct Node* createNode(void *data){
    struct Node *newNode = (struct Node *)malloc(sizeof(struct Node));
    if (!newNode) {
        perror("malloc: ");
        exit(1);
    }
    newNode->data = data;
    newNode->next = NULL;
    return newNode; 
}

struct List* createList(){
    struct List *list = (struct List *)malloc(sizeof(struct List));
    if (!list) {
        perror("malloc: ");
        exit(1);
    }
    list->head = NULL;
    list->size = 0;
    return list;
}

void insertAtFront(struct List* list, void * data){
    struct Node *newNode = createNode(data);
    newNode->next = list->head;
    list->head = newNode;
    list->size++;
}

void freeList(struct List* list){
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
