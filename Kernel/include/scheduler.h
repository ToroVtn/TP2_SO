#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

typedef enum { READY, BLOCKED, TERMINATED } ProcessState;

typedef struct ProcessControlBlock {
    uint32_t pid;                // Process ID
    uint8_t priority;           // Process priority      
    ProcessState state;          // Current state of the process
    void* rsp;      // Pointer to the process's stack
    void* rbp;
    char* name;
    int code;
    int waitedCode; // Code that the process waited for
} ProcessControlBlock;

void initPCBlist();
uint32_t createProcess(int argc, char* argv[], void* processRip);
void* schedule(void* rsp);
void exitSwitcher();
void exit(int code);
int waitPid(uint32_t pid);
//void startFirstProcess(void* processAddress);
#endif