#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

typedef enum { READY, RUNNING, BLOCKED, TERMINATED } ProcessState;

typedef struct {
    uint32_t pid;                // Process ID
    uint8_t priority;           // Process priority      
    ProcessState state;          // Current state of the process
    void *stackPointer;      // Pointer to the process's stack
} ProcessControlBlock;

void initPCBlist();
uint32_t createProcess(int argc, char* argv[], void* rip);
void* schedule(void* rsp);
extern void exit(int code);

#endif