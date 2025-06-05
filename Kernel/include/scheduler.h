#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_NAME_LENGTH 60
#define KILL_CODE 1

typedef enum { READY, RUNNING, BLOCKED, EXITED, STANDBY_FOR_EXIT } State;
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
  struct PCB* parent;
  int waitingPCBCount;
  void* stack;
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
extern void exit(int exitCode);
void startFirstProcess(void* procAddress);
int waitPid(uint32_t pid);
userlandPCB* fetchPCBList(int* len);
const PCB* fetchCurrentPCB();
void blockProc();
void readyProc(const PCB* pcb);
uint32_t getpid();
bool kill(uint32_t pid);
void killCurrentForegroundProcess();
void changePriority(uint32_t pid, uint8_t newPriority);
void exitProcessByPCB(PCB* pcb, int exitCode);

#endif