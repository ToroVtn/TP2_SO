#include "pipes.h"
#include "semaphore.h"


#define MAX_PIPES 10


int pipeCount;
PipeSlot pipeArray[MAX_PIPES];

//Chequea si el pipe es válido, devuelve 1 si no es válido, 0 si es válido
int validPipe(int pipe) {
    return (pipe < 0 || pipe >= MAX_PIPES || !pipeArray[pipe].available);
}

void initPipes() {
    for (int i = 0; i < MAX_PIPES; i++) {
        pipeArray[i].available = 0;
    }
    pipeCount = 0;
}

int findOpenSlot() {
    if (pipeCount >= MAX_PIPES) {
        return -1;
    }
    for (int i = 0; i < MAX_PIPES; ++i) {
        if (!pipeArray[i].available) {
            return i;
        }
    }
    return -1;
}

int lookupPipe(const char* name) {
    for (int i = 0; i < MAX_PIPES; i++) {
        if (pipeArray[i].available && strCmp(pipeArray[i].pipe->name, name) == 0) {
            return i;
        }
    }
    return -1;
}

// Add this helper function to create semaphore names safely
char* createSemName(const char* prefix, const char* pipeName) {
    int prefixLen = strlen(prefix);
    int nameLen = strlen(pipeName);
    char* semName = malloc(prefixLen + nameLen + 1);
    
    if (semName == NULL) return NULL;
    
    strcpy(semName, prefix);
    strcat(semName, pipeName);
    
    return semName;
}
//Si no quiero asignar el proceso todavía le mando 0 y con el join lo puedo agregar
int createPipe(char* name, int process_write, int process_read) {
    if (process_write < 0 || process_read < 0) {
        return -1;
    }
    if (lookupPipe(name) != -1) {
        return -1;
    }
    int available_pipe = findOpenSlot();
    if (available_pipe == -1) {
        return -1;
    }
    pipeArray[available_pipe].available = 1;
    pipeCount++;
    pipeArray[available_pipe].pipe = malloc(sizeof(pipe));
    if (pipeArray[available_pipe].pipe == NULL) {
        pipeArray[available_pipe].available = 0;
        pipeCount--;
        return -1;
    }

    char* mutexName = createSemName("pipe_m_", name);
    char* emptyName = createSemName("pipe_e_", name);
    char* fullName = createSemName("pipe_f_", name);

    pipeArray[available_pipe].pipe->mutex = initSem(mutexName, 1);
    pipeArray[available_pipe].pipe->empty = initSem(emptyName, BUFFER_SIZE);
    pipeArray[available_pipe].pipe->full = initSem(fullName, 0);
    
    /* strcpy(pipeArray[available_pipe].pipe->name, name);
    pipeArray[available_pipe].pipe->producingIndex = 0;
    pipeArray[available_pipe].pipe->consumingIndex = 0;
    pipeArray[available_pipe].pipe->mutex = initSem(strcat("pipe_m_", name), 1);
    pipeArray[available_pipe].pipe->empty = initSem(strcat("pipe_e_", name), BUFFER_SIZE);
    pipeArray[available_pipe].pipe->full = initSem(strcat("pipe_f_", name), 0);
 */

    pipeArray[available_pipe].pipe->read = process_read;
    pipeArray[available_pipe].pipe->write = process_write;
    return available_pipe;
}

int attachToPipe(const char* pipe_name, int process_write, int process_read) {
    int pipe_id = lookupPipe(pipe_name);
    if (pipe_id == -1) {
        return -1;
    }
    if (pipeArray[pipe_id].pipe->write == 0 && process_write > 0) {
        pipeArray[pipe_id].pipe->write = process_write;
    } else if (process_write != 0) {
        return -1;
    }
    if (pipeArray[pipe_id].pipe->read == 0 && process_read > 0) {
        pipeArray[pipe_id].pipe->read = process_read;
    } else if (process_read != 0) {
        return -1;
    }
    return 1;
}

int writeToPipe(int pipe, const char* info, int size) {
    if (validPipe(pipe)) {
        return -1;
    }
    if (pipeArray[pipe].pipe->write != 0 && pipeArray[pipe].pipe->read != 0) {
        int pos = 0;
        while (pos < size) {
            semWait(pipeArray[pipe].pipe->empty);
            semWait(pipeArray[pipe].pipe->mutex);
            pipeArray[pipe].pipe->buffer[pipeArray[pipe].pipe->producingIndex] = info[pos++];
            pipeArray[pipe].pipe->producingIndex = (pipeArray[pipe].pipe->producingIndex + 1) % BUFFER_SIZE;
            semPost(pipeArray[pipe].pipe->mutex);
            semPost(pipeArray[pipe].pipe->full);
        }
        return pos;
    }
    return -1;
}

int readFromPipe(int pipe, char* info, int size) {
    if (validPipe(pipe)) {
        return -1;
    }
    if (pipeArray[pipe].pipe->write != 0 && pipeArray[pipe].pipe->read != 0) {
        int pos = 0;
        while (pos < size) {
            semWait(pipeArray[pipe].pipe->full);
            semWait(pipeArray[pipe].pipe->mutex);
            info[pos++] = pipeArray[pipe].pipe->buffer[pipeArray[pipe].pipe->consumingIndex];
            pipeArray[pipe].pipe->consumingIndex = (pipeArray[pipe].pipe->consumingIndex + 1) % BUFFER_SIZE;
            semPost(pipeArray[pipe].pipe->mutex);
            semPost(pipeArray[pipe].pipe->empty);
        }
        return pos;
    }
    return -1;
}

int deletePipe(int pipe) {
    if (validPipe(pipe)) {
        return -1;
    }
    free(pipeArray[pipe].pipe->name);
    free(pipeArray[pipe].pipe);
    pipeArray[pipe].available = 0;
    pipeCount--;
    return 0;
}