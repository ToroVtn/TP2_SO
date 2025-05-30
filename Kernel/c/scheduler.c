#include "../include/scheduler.h"
#include <stddef.h>

extern void* createStack(int argc, char* argv[], void* rip);
extern void* idleProcess();

typedef struct PCBnode {
    ProcessControlBlock* pcb;
    ProcessControlBlock* waitingProcesses[20];
    int waitingCount;
    struct PCBnode* next;
} PCBnode;

typedef struct {
    PCBnode* head;
    PCBnode* tail;
    PCBnode* current;
    PCBnode* previous;
} PCBlist;

static uint32_t pid = 0;

PCBnode idleProcessPCB;
PCBlist pcbList;



ProcessControlBlock* createPCB(uint32_t pid, uint8_t priority, ProcessState state, void* rsp) {
  ProcessControlBlock* pcb = malloc(sizeof(ProcessControlBlock));
  pcb->pid = pid;
  pcb->priority = priority;
  pcb->state = state;
  pcb->rsp = rsp;

  return pcb;
}

PCBnode* createPCBNode(uint32_t pid, uint8_t priority, ProcessState state, void* rsp) {
  PCBnode* node = malloc(sizeof(PCBnode));
  node->next = NULL;
  node->pcb = createPCB(pid, priority, state, rsp);
  node->waitingCount = 0;

  return node;
}

void freeProcessNode(PCBnode* node) {
  free(node->pcb);
  free(node);
}

void initProcList() {
  void* rsp = createStack(0, NULL, idleProcess);  
  idleProcessPCB.pcb->pid = pid;
  idleProcessPCB.pcb->priority = 0;
  idleProcessPCB.pcb->state = READY;
  idleProcessPCB.pcb->rsp = rsp;

  pcbList.head = NULL;
  pcbList.tail = NULL;
  pcbList.current = NULL;
  pcbList.previous = NULL;

  // pcbList.len = 0;
}

uint32_t addPCB(void* rsp) {
  PCBnode* node = createPCBNode(pid, 1, READY, rsp);

  if (pcbList.tail == NULL) {
    pcbList.head = node;
    pcbList.tail = node;
  } else {
    pcbList.tail->next = node;
    pcbList.tail = node;
  }

  return pid++;
  // ++pcbList.len;
  // return node->pcb;
}

void* schedule(void* rsp) {
  static int n = 0;

  if (pcbList.head == NULL) return idleProcessPCB.pcb->rsp;

  if (pcbList.current == NULL) {
    pcbList.current = pcbList.head;
  }

  pcbList.current->pcb->rsp = rsp;

  if (n > 0 && pcbList.current->pcb->state == READY) {
    --n;
    return rsp;
  }

  while (pcbList.current != NULL && pcbList.current->pcb->state != READY) {
    pcbList.previous = pcbList.current;
    pcbList.current = pcbList.current->next;
  }

  if (pcbList.current != NULL) {
    n = pcbList.current->pcb->priority - 1;
    return pcbList.current->pcb->rsp;
  } else {
    return idleProcessPCB.pcb->rsp;
  }
}

uint32_t createProcess(int argc, char* argv[], void* processRip) {
  void* rsp = createProcessStack(argc, argv, processRip); 
  return addPCB(rsp);
}

void exitProcess(int exitCode) {
  

  for (int i = 0; i < pcbList.current->waitingCount; ++i) {
    ProcessControlBlock* pcb = pcbList.current->waitingProcesses[i];
    pcb->state = READY;
  }

  // pcbList.current->pcb->state = EXITED;
  if (pcbList.current == pcbList.head) {
    pcbList.head = pcbList.head->next;
    freePCBNode(pcbList.current);
    pcbList.current = pcbList.head;
  } else {
    
    // This should never happen and should be removed after testing...
    // I'm using it so I get an exception in case this isn't working as it should.
    //if (pcbList.previous == NULL) 1 / 0;

    pcbList.previous->next = pcbList.current->next;
    freePCBNode(pcbList.current);
    pcbList.current = pcbList.previous->next;
  }
}

void waitPid(uint32_t pid) {
  PCBnode* node = pcbList.head;
  // Note pcbList is orded by pid because new nodes are always added at the end and
  // pid is always increasing..
  while (node != NULL && node->pcb->pid <= pid) {
    if (node->pcb->pid == pid) {
      node->waitingProcesses[node->waitingCount++] = pcbList.current->pcb;
      pcbList.current->pcb->state = BLOCKED;
      return;
    }
    node = node->next;
  }
  // If no process with the specified pid is found then current process is not blocked.
}
