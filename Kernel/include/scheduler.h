#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

#define MAX_NAME_LENGTH 60

typedef enum { READY, RUNNING, BLOCKED, EXITED } State;
extern const char* const StateStrings[4];

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

typedef struct {
  uint32_t pid;
  uint8_t priority;
  const char* state;
  void* rsp;
  void* rbp;
  char name[MAX_NAME_LENGTH + 1];
} PCBForUserland;

// void freePCBNode(PCBNode* node);
void initializePCBList();
void* schedule(void* rsp);
uint32_t createUserProcess(int argc, char* argv[], void* processRip);
extern void exit(int exitCode);
void startFirstProcess(void* processAddress);
int waitPid(uint32_t pid);
PCBForUserland* getPCBList(int* len);

#endif