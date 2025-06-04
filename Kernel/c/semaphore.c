#include <semaphores.h>
#include <scheduler.h>
#include <utils.h>

#define ERROR (-1)

semSlot semArray[MAX_SEMAPHORES];
int arraySize;


//chequea si encuentra el valor del semaforo dentro de mi arreglo

int initSemArray(){
    for (int i = 0; i < MAX_SEMAPHORES; ++i) {
        semArray[i].used=0;
    }
    arraySize = 0;
    return 1;
}

int findSem(char *name) {
    if(!arraySize) {
        return ERROR; //No hay semáforos
    }
    for (int i=0; i < MAX_SEMAPHORES; i++) {
        if (semArray[i].used) {
            if (strCmp(semArray[i].sem->name, name) == 0)
                return i;
        }
    }
    return ERROR;
}

//Devuelve una posicion para iniciar un nuevo semaforo, si falla retorna -1
int findSemSlot() {
    for (int i = 0; i < MAX_SEMAPHORES; i++) {
        if (!semArray[i].used) {
            return i;
        }
    }
    return ERROR;
}


int queue(int pos, const PCB* queuedProcess) {
    queuedProc* process = malloc(sizeof(queuedProc));
    if (process==NULL){
        return ERROR;
    }
    process->procPCB = queuedProcess;
    process->next = NULL;
    process->previous = NULL;
    if (semArray[pos].sem->firstProc == NULL) {
        semArray[pos].sem->firstProc = process;
        semArray[pos].sem->lastProc = process;
    } else{
        process->next=NULL;
        process->previous=semArray[pos].sem->lastProc;
        semArray[pos].sem->lastProc->next = process;
        semArray[pos].sem->lastProc = process;
    }
    return 0;
}


const PCB* dequeue(int pos) {
    if (semArray[pos].sem->firstProc == NULL)
        return NULL;
    const PCB* process = semArray[pos].sem->firstProc->procPCB;
    queuedProc *temp = semArray[pos].sem->firstProc;
    if (semArray[pos].sem->firstProc->next == NULL) {
        semArray[pos].sem->firstProc = NULL;
        semArray[pos].sem->lastProc = NULL;
    } else {
        semArray[pos].sem->firstProc = semArray[pos].sem->firstProc->next;
        semArray[pos].sem->firstProc->previous = NULL;
    }
    free(temp);
    return process;
}

int createSemaphore(char* name, int value ){
    return initSem(name, value);
}
int destroySemaphore(char* name){
    return closeSem(findSem(name));
}
int postSemaphore(int semId){
    return semPost(semId);
}
int waitSemaphore(int semId){
    return semWait(semId);
}

int openSemaphore(char* name, int value) {
    return openSem(name, value);
}

int initSem(char *name, unsigned int init_value) {
    if (findSem(name)!=ERROR)
        return ERROR;
    int pos = findSemSlot();
    if (pos == ERROR) {
        return ERROR;
    }
    semArray[pos].sem = malloc(sizeof(semaphore));
    if (semArray[pos].sem==NULL){
        return ERROR;
    }
   semArray[pos].sem->name = malloc(strlen(name) + 1);
    if (semArray[pos].sem->name == NULL) {
        free(semArray[pos].sem);
        return ERROR;
    }
    strcpy(semArray[pos].sem->name, name);
    semArray[pos].sem->value = init_value;
    semArray[pos].sem->lock=0;
    semArray[pos].used = 1;
    semArray[pos].sem->firstProc = NULL;
    semArray[pos].sem->lastProc = NULL;
    arraySize++;
    return pos;
}

/*--------------------------------------------*/




int openSem(char *name, int value) {
    int semId = findSem(name);
    if (semId == ERROR && value != -1) {
        semId = initSem(name, value);
        if (semId == ERROR)
            return ERROR;
    }
    return semId;
}

int closeSem(int semId) {
    if (semId >= MAX_SEMAPHORES || semId < 0)
        return ERROR;
    enterCritical(&semArray[semId].sem->lock);
    if(semArray[semId].sem->firstProc != NULL) {
        //Si hay procesos esperando, no puedo eliminar el semaforo
        return ERROR;
    }
    exitCritical(&semArray[semId].sem->lock);
    free(semArray[semId].sem->name);
    free(semArray[semId].sem);
    semArray[semId].used = 0;
    arraySize--;
    return 0;
}
//Busca el semaforo, si no lo encuentra sale. Si lo encuentra le decrementa el valor (si es 0 lo manda a la cola del sem)
//y le dice al scheduler que lo bloquee
int semWait(int semId) {
    if (semId>=MAX_SEMAPHORES || semId < 0 || !semArray[semId].used) {
        return ERROR;
    }
    enterCritical(&semArray[semId].sem->lock);
    if (semArray[semId].sem->value > 0) {
        semArray[semId].sem->value--;
        exitCritical(&semArray[semId].sem->lock);
    }
    //lo tengo que encolar porque tiene que ponerse a esperar
    else {
        const PCB* procPCB = fetchCurrentPCB();
        queue(semId, procPCB);
        exitCritical(&semArray[semId].sem->lock);
        blockProc();
    }
    return 0;
}
//Busca el semaforo, si no lo encuentra sale. Si lo encuentra le aumenta el valor y se fija si hay algún elemento que se
//necesite desencolar. Si hay alguno le dice al scheduler que lo pase a ready
int semPost(int semId) {
    if (semId>=MAX_SEMAPHORES || semId <0 || !semArray[semId].used) {
        return ERROR;
    }
    enterCritical(&semArray[semId].sem->lock);
    if (semArray[semId].sem->firstProc!=NULL) {
        const PCB* to_ready=dequeue(semId);
        exitCritical(&semArray[semId].sem->lock);
        readyProc(to_ready);
    }else {
        semArray[semId].sem->value++;
        exitCritical(&semArray[semId].sem->lock);
    }
    return 0;
}