#ifndef SEMPAPHORES_SEMAPHORE_H
#define SEMPAPHORES_SEMAPHORE_H

#include <memory.h>
#include "semaphores.h"
#include <scheduler.h>
#include <utils.h>
#include <stdbool.h>

#define MAX_NAME_LENGTH 50


extern int enterCritical(int *lock);
extern int exitCritical(int *lock);

typedef struct PCBNodeSem{
    const PCB* procPCB;
    struct PCBNodeSem* next;
    struct PCBNodeSem* previous;
}PCBNodeSem;

typedef struct semaphore{
    int value;
    int lock;
    bool destroyed;
    PCBNodeSem * pcbNodeHead;
    PCBNodeSem * pcbNodeTail;
    char* name[MAX_NAME_LENGTH + 1];
} semaphore;


int findSem(char* name);

int initSemArray();
int initSem(unsigned int value);

int createSem(char* name, int value);
bool destroySemaphore(int semId);
bool destroySemaphoreByName(char* name);
bool postSemaphore(int semId);
bool waitSemaphore(int semId);
int openSemaphore(char* name, int value);
bool decSemOnlyForKernel(int semId);
#endif //SEMPAPHORES_SEMAPHORE_H