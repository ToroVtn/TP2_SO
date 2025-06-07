#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <memory.h>
#include <stdbool.h>

#define PROCESS_HEAP_ORDER_COUNT 17
#define PROCESS_HEAP_SIZE (1 << (PROCESS_HEAP_ORDER_COUNT - 1))

#define MAX_NAME_LENGTH 50
#define KILL_CODE 1

typedef enum { READY, RUNNING, BLOCKED, EXITED, STANDBY_FOR_EXIT, USER_BLOCKED } State;
extern const char* const stateNames[5];

typedef struct {
  int32_t write;
  int32_t read;
  int32_t err;
} ProcessPipes;

#define PROCESS_HEAP_ORDER_COUNT 17
#define PROCESS_HEAP_SIZE (1 << (PROCESS_HEAP_ORDER_COUNT - 1))


typedef struct PCB {
  uint32_t pid;
  uint8_t priority;
  State state;
  void* rsp;
  void* rbp;
  char* name;
  int waitedProcCode;
  struct PCB* waitingPCBs[10]; 
  struct PCB* parent;
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
  ProcessPipes pipes;
} PCB;

typedef struct {
  uint32_t pid;
  uint8_t priority;
  const char* state;
  void* rsp;
  void* rbp;
  char name[MAX_NAME_LENGTH + 1];
  char* location;
} userlandPCB;


void createPCBList();
void* schedule(void* rsp);
uint32_t initUserProc(int argc, char* argv[], void* procRip);
int32_t initUserProcWithPipeSwap(int32_t argc, const char* argv[], void* processRip, ProcessPipes pipes);
extern void exit(int exitCode);
void startFirstProcess(void* procAddress);
void exitProc(int exitCode);
int waitPid(uint32_t pid);
userlandPCB* fetchPCBList(int* len);
PCB* fetchCurrentPCB();
void blockProc();
void readyProc(const PCB* pcb);
uint32_t getpid();
bool kill(uint32_t pid);
void killCurrentForegroundProcess();
void setPriority(uint32_t pid, uint8_t newPriority);
void changePipeRead(int32_t p);
void changePipeWrite(int32_t p);
void blockByUser(uint32_t pid);
ProcessPipes fetchPipes();
void exitProcessByPCB(PCB* pcb, int exitCode);
void block(uint32_t pid);
void unBlock(uint32_t pid);
void yield();
int64_t read(int32_t pipeId, char* buf, int32_t len);
int64_t write(int32_t pipeId, const char* buf, int32_t len);

#endif