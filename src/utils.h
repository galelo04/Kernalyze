#ifndef UTILS_H
#define UTILS_H
#include "defs.h"

//linked list node and list definitions for stroing the pcb_table
struct NodePCB{
    struct PCB data;
    struct NodePCB* next;
};

struct ListPCB
{
    struct NodePCB* head;
    int size;
};


struct NodePCB * createNode(struct PCB data);

struct ListPCB * createList();

void insertAtFront(struct ListPCB * list , struct PCB data);

void freeList(struct ListPCB * list);

struct PCB * findPCB(struct ListPCB * list , pid_t pid);




#endif