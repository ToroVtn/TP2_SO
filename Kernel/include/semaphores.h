#ifndef SEMPAPHORES_SEMAPHORE_H
#define SEMPAPHORES_SEMAPHORE_H

#include "stdlib.h"
#include "semaphores.h"
#include "scheduler.h"
#include "utils.h"

#define MAX_SEMAPHORES 50


extern int enterCritical(int *lock);
extern int exitCritical(int *lock);

typedef struct queuedProc{
    const PCB* procPCB;
    struct queuedProc* next;
    struct queuedProc* previous;
}queuedProc;

typedef struct semaphore{
    char* name;
    int value;
    int lock;
    queuedProc * firstProc;
    queuedProc * lastProc;
} semaphore;

typedef struct semSlot {
    semaphore *sem;
    int used;
} semSlot;



int findSem(char* name);
int findSemSlot();

int initSemArray();
int initSem(char* name, unsigned int value);
int semWait(int semId);
int semPost(int semId);
int openSem(char* name, int value);
int closeSem(int semId);


int createSemaphore(char* name, int value);
int destroySemaphore(char* name);
int postSemaphore(int semId);
int waitSemaphore(int semId);
int openSemaphore(char* name, int value);
#endif //SEMPAPHORES_SEMAPHORE_H