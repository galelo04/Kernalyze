#include "utils.h"
#include <stddef.h>
#include <stdio.h>
#include <errno.h>



//implementation of list
struct NodePCB * createNode(struct PCB data){
    struct NodePCB * newNode = (struct NodePCB *)malloc(sizeof(struct NodePCB));
    if (!newNode) {
        perror("malloc: ");
        exit(1);
    }
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}
struct ListPCB * createList(){
    struct ListPCB * list = (struct ListPCB*)malloc(sizeof(struct ListPCB));
    if (!list) {
        perror("malloc: ");
        exit(1);
    }
    list->head = NULL;
    list->size = 0;
    return list;
}
void insertAtFront(struct ListPCB * list , struct PCB data){
    struct NodePCB * newNode = createNode(data);
    newNode->next = list->head;
    list->head = newNode;
    list->size++;
}
void freeList(struct ListPCB * list){
    struct NodePCB * temp;
    while(list->head){
        temp = list->head;
        list->head  = list->head->next;
        free(temp);
    }
    list->size = 0;
    free(list);
}

struct PCB * findPCB(struct ListPCB * list , pid_t pid){
    struct NodePCB * temp  = list->head;
    while(temp){
        if(temp->data.pid == pid)
            return &(temp->data);
    }
    return NULL;
}