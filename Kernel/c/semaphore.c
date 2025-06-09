#include <semaphores.h>
#include <arrayUtils.h>
#include <scheduler.h>
#include <utils.h>

#define ERROR (-1)
#define INITIAL_CAPACITY 20

uint32_t size;
static Array semArray; //Arreglo de semáforos
static Array freedPositions; //Arreglo de posiciones libres

//chequea si encuentra el valor del semaforo dentro de mi arreglo

int initSemArray(){
    semArray = initArray(sizeof(semaphore), INITIAL_CAPACITY, NULL);
    freedPositions = initArray(sizeof(int32_t), INITIAL_CAPACITY, NULL);
}

int addSem(char* name, uint32_t initialValue) {
    semaphore sem;
    sem.value = initialValue;
    sem.lock = 0;
    sem.pcbNodeHead = NULL;
    sem.pcbNodeTail = NULL;
    sem.destroyed = false;
    int32_t i;
    for (i = 0; i < MAX_NAME_LENGTH && name[i] != 0; ++i) {
        sem.name[i] = name[i];
    }
    if (name[i] != 0) return -1;
    int32_t freeToUseSem;
    if (popAndGetFromArray(freedPositions, &freeToUseSem)) {
        setAtArrayIdx(semArray, freeToUseSem, &sem);
        return freeToUseSem;
    } else {
        return pushToArray(semArray, &sem);
  }
}

int initSem(unsigned int value) {
    return addSem("", value);
}

int findSem(char *name) {
    int32_t len = arrayLen(semArray);
  for (int32_t i = 0; i < len; ++i) {
    semaphore* sem = getAtArrayIdx(semArray, i);
    if (!sem->destroyed) {
      if (strcmp(name, sem->name) == 0) return i;
    }
  }
  return -1;
}


int queue(int semId, const PCB* queuedProcess) {
    semaphore* sem = getAtArrayIdx(semArray, semId);
    if (sem == NULL) {
        return false;
    }

    PCBNodeSem* node = globalMalloc(sizeof(PCBNodeSem));
    if (node == NULL) {
        return false;
    }
    node->procPCB = queuedProcess;
    node->next = NULL;
    if (sem->pcbNodeHead == NULL) {
        sem->pcbNodeHead = node;
        sem->pcbNodeTail = node;
    } else {
        sem->pcbNodeTail->next = node;
        sem->pcbNodeTail = node;
    }
  return true;
}


const PCB* dequeue(int semId) {
    semaphore* sem = getAtArrayIdx(semArray, semId);
    if (sem == NULL || sem->pcbNodeHead == NULL) {
        return NULL;
    }
    PCB* pcb = sem->pcbNodeHead->procPCB;
    PCBNodeSem* temp = sem->pcbNodeHead;
    sem->pcbNodeHead = sem->pcbNodeHead->next;
    globalFree(temp);
    return pcb;
}

int createSem(char* name, int initialValue) {
    if(findSem(name) >= 0) {
        return ERROR; 
    }
    return addSem(name, initialValue);
}
bool destroySemaphore(int semId) {
    if (semId < 0) return false;
    semaphore* sem = getAtArrayIdx(semArray, semId);
    if (sem == NULL || sem->destroyed) return false;
    enterCritical(&sem->lock);
    while (sem->pcbNodeHead != NULL) {
        PCB* toReady = dequeue(semId);
        exitCritical(&sem->lock);
        if (toReady->state == BLOCKED) {
        readyProc(toReady);
        } else if (toReady->state == STANDBY_FOR_EXIT) {
        exitProcessByPCB(toReady, KILL_CODE);
        }
    }
    exitCritical(&sem->lock);
    sem->destroyed = true;
    pushToArray(freedPositions, &semId);
    return true;
}

bool destroySemaphoreByName(char* name) {
  return destroySemaphore(findSem(name));
}

bool postSemaphore(int semId){
    if (semId < 0) return false;
    semaphore* sem = getAtArrayIdx(semArray, semId);
    if (sem == NULL || sem->destroyed) return false;
    enterCritical(&sem->lock);

    bool shouldLeave = false;
    while (!shouldLeave) {
        PCB* toReady = dequeue(semId);
        if (toReady != NULL) {
        if (toReady->state == BLOCKED) {
            readyProc(toReady);
            shouldLeave = true;
        } else if (toReady->state == STANDBY_FOR_EXIT) {
            exitProcessByPCB(toReady, KILL_CODE);
        }
        } else {
        sem->value++;
        shouldLeave = true;
        }
    }

    exitCritical(&sem->lock);

    return true;
}

bool waitSemaphore(int semId){
    if (semId < 0) return false;
    semaphore* sem = getAtArrayIdx(semArray, semId);
    if (sem == NULL || sem->destroyed) return false;
    enterCritical(&sem->lock);
    if (sem->value > 0) {
        sem->value--;
        exitCritical(&sem->lock);
    } else {
        PCB* pcb = fetchCurrentPCB();
        queue(semId, pcb);
        exitCritical(&sem->lock);
        blockProc();
    }
    return true;
}

bool decSemOnlyForKernel(int32_t semId) {
  if (semId < 0) return false;
  semaphore* sem = getAtArrayIdx(semArray, semId);
  if (sem->value > 0) sem->value--;
  return true;
}

int openSemaphore(char* name, int value) {
    int semId = findSem(name);
    if (semId < 0) semId = addSem(name, value);
    return semId;
}