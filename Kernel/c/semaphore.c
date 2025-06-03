#include "semaphores.h"
#include "scheduler.h"
#include "utils.h"


//Supongamos que un semáforo se comporta como una lista
//un arreglo con todos los elems inicializados y si estan
//siendo usados me importa que pasa con ellos, si no los
//dejo vacíos y los puedo ir ocupando y vaciando a medida
//que necesite


//Los semaforos no los puede gestionar cada proceso como tal
//porque tienen que ser accesibles por varios procesos entonces
//hay que usar nombre

//usando solamente indices puedo encapsular los semaforos de forma que solo se puedan acceder desde aca
//en cada llamada se debe usar el nombre del semaforo y a la vez el pid del proceso que lo llama





#define ERROR (-1)

semSlot sem_array[MAX_SEMAPHORES];

/*-----------AUX. FUNCTIONS-------------------*/ //FUNCIIONES QUE NO ESTÁN EN LOS TEST
//Se fija si encuentra el valor del semaforo dentro de mi arreglo. Si falla -1

int initSemArray(){
    for (int i = 0; i < MAX_SEMAPHORES; ++i) {
        sem_array[i].used=0;
    }
    return 1;
}

int findSem(char *name) {
    for (int i=0; i < MAX_SEMAPHORES; i++) {
        if (sem_array[i].used) {
            if (s_strcmp(sem_array[i].sem->name, name) == 0)
                return i;
        }
    }
    return ERROR;
}

//Devuelve una posicion para iniciar un nuevo semaforo, si falla retorna -1
int findSemSlot() {
    for (int i = 0; i < MAX_SEMAPHORES; i++) {
        if (!sem_array[i].used) {
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
    if (sem_array[pos].sem->firstProc == NULL) {
        sem_array[pos].sem->firstProc = process;
        sem_array[pos].sem->lastProc = process;
    } else{
        process->next=NULL;
        process->previous=sem_array[pos].sem->lastProc;
        sem_array[pos].sem->lastProc->next = process;
        sem_array[pos].sem->lastProc = process;
    }
    return 0;
}


const PCB* dequeue(int pos) {
    if (sem_array[pos].sem->firstProc == NULL)
        return NULL;
    const PCB* process = sem_array[pos].sem->firstProc->procPCB;
    queuedProc *temp = sem_array[pos].sem->firstProc;
    if (sem_array[pos].sem->firstProc->next == NULL) {
        sem_array[pos].sem->firstProc = NULL;
        sem_array[pos].sem->lastProc = NULL;
    } else {
        sem_array[pos].sem->firstProc = sem_array[pos].sem->firstProc->next;
        sem_array[pos].sem->firstProc->previous = NULL;
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
    sem_array[pos].sem = malloc(sizeof(semaphore));
    if (sem_array[pos].sem==NULL){
        return ERROR;
    }
   sem_array[pos].sem->name = malloc(s_strlen(name) + 1);
    if (sem_array[pos].sem->name == NULL) {
        free(sem_array[pos].sem);
        return ERROR;
    }
    strcpy(sem_array[pos].sem->name, name);
    sem_array[pos].sem->value = init_value;
    sem_array[pos].sem->lock=0;
    sem_array[pos].used = 1;
    sem_array[pos].sem->firstProc = NULL;
    sem_array[pos].sem->lastProc = NULL;
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
    enterCritical(&sem_array[semId].sem->lock);
    if(sem_array[semId].sem->firstProc != NULL) {
        //Si hay procesos esperando, no puedo eliminar el semaforo
        return ERROR;
    }
    exitCritical(&sem_array[semId].sem->lock);
    free(sem_array[semId].sem->name);
    free(sem_array[semId].sem);
    sem_array[semId].used = 0;
    return 0;
}
//Busca el semaforo, si no lo encuentra sale. Si lo encuentra le decrementa el valor (si es 0 lo manda a la cola del sem)
//y le dice al scheduler que lo bloquee
int semWait(int semId) {
    if (semId>=MAX_SEMAPHORES || semId < 0 || !sem_array[semId].used) {
        return ERROR;
    }
    enterCritical(&sem_array[semId].sem->lock);
    if (sem_array[semId].sem->value > 0) {
        sem_array[semId].sem->value--;
        exitCritical(&sem_array[semId].sem->lock);
    }
    //lo tengo que encolar porque tiene que ponerse a esperar
    else {
        const PCB* procPCB = fetchCurrentPCB();
        queue(semId, procPCB);
        exitCritical(&sem_array[semId].sem->lock);
        blockProc();
    }
    return 0;
}
//Busca el semaforo, si no lo encuentra sale. Si lo encuentra le aumenta el valor y se fija si hay algún elemento que se
//necesite desencolar. Si hay alguno le dice al scheduler que lo pase a ready
int semPost(int semId) {
    if (semId>=MAX_SEMAPHORES || semId <0 || !sem_array[semId].used) {
        return ERROR;
    }
    enterCritical(&sem_array[semId].sem->lock);
    if (sem_array[semId].sem->firstProc!=NULL) {
        const PCB* to_ready=dequeue(semId);
        exitCritical(&sem_array[semId].sem->lock);
        readyProc(to_ready);
    }else {
        sem_array[semId].sem->value++;
        exitCritical(&sem_array[semId].sem->lock);
    }
    return 0;
}