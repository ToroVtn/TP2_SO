#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

typedef enum { READY, RUNNING, BLOCKED, EXITED } State;

typedef struct PCB {
  uint32_t pid;
  uint8_t priority;
  State state;
  void* rsp;
  void* rbp;
  char* name;
  // int exitCode;
  int waitedProcessExitCode;
  struct PCB* waitingForMe[10]; // This should be of dynamic length
  int wfmLen;
  void* stack;
} PCB;

// void freePCBNode(PCBNode* node);
void initializePCBList();
void* schedule(void* rsp);
uint32_t createUserProcess(int argc, char* argv[], void* processRip);
extern void exit(int exitCode);
void startFirstProcess(void* processAddress);
int waitPid(uint32_t pid);

#endif