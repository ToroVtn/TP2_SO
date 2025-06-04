#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <memory.h>
#include <stdbool.h>

#define PROCESS_HEAP_ORDER_COUNT 17
#define PROCESS_HEAP_SIZE (1 << (PROCESS_HEAP_ORDER_COUNT - 1))

#define MAX_NAME_LENGTH 60

typedef enum { READY, RUNNING, BLOCKED, EXITED } State;
extern const char* const stateNames[4];

typedef struct PCB {
  uint32_t pid;
  uint8_t priority;
  State state;
  void* rsp;
  void* rbp;
  char* name;
  int waitedProcCode;
  struct PCB* waitingPCBs[10]; 
  int waitingPCBCount;
  void* stack;
  void* heap;
  bool heapFreed;
#ifdef BUDDY
  Block* freeList[PROCESS_HEAP_ORDER_COUNT];
#else
  Block* listStart;
  Block* listEnd;
  size_t freeBytes;
#endif
} PCB;

typedef struct {
  uint32_t pid;
  uint8_t priority;
  const char* state;
  void* rsp;
  void* rbp;
  char name[MAX_NAME_LENGTH + 1];
} userlandPCB;


void createPCBList();
void* schedule(void* rsp);
uint32_t initUserProc(int argc, char* argv[], void* procRip);
extern void exit(int exitCode);
void startFirstProcess(void* procAddress);
int waitPid(uint32_t pid);
userlandPCB* fetchPCBList(int* len);
PCB* fetchCurrentPCB();
void blockProc();
void readyProc(PCB* pcb);

#endif