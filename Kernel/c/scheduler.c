#include "../include/scheduler.h"
#include <stddef.h>
#include <memory.h>
#include <stdlib.h>

extern void* createStack(int argc, char* argv[], void* rip, void* stackBase);
extern void* idleProcess();
extern void* userModule();
extern void switcherInterruption();


typedef struct PCBnode {
    ProcessControlBlock* pcb;
    ProcessControlBlock* waitingProcesses[20];
    int waitingCount;
    void* stack;
    struct PCBnode* next;
} PCBnode;

typedef struct {
    PCBnode* head;
    PCBnode* tail;
    PCBnode* current;
    PCBnode* previous;
} PCBlist;


static int quantumsLeft = 0;
PCBnode idleProcessPCB;
PCBlist pcbList;



ProcessControlBlock* createPCB(uint32_t pid, uint8_t priority, ProcessState state, void* rsp, void* rbp, char* name) {
  ProcessControlBlock* pcb = malloc(sizeof(ProcessControlBlock));
  pcb->pid = pid;
  pcb->priority = priority;
  pcb->state = state;
  pcb->rsp = rsp;
  pcb->rbp = rbp;
  pcb->name = name;
  pcb->code =0;

  return pcb;
}

PCBnode* createPCBNode(uint32_t pid, uint8_t priority, ProcessState state, void* rsp, void* stack, void* rbp, char* name) {
  PCBnode* node = malloc(sizeof(PCBnode));
  node->next = NULL;
  node->pcb = createPCB(pid, priority, state, rsp, rbp, name);
  node->waitingCount = 0;
  node->stack = stack;

  return node;
}

void freeProcessNode(PCBnode* node) {
  free(node->pcb);
  free(node->stack);
  free(node);
}

void initProcList() {
  void* stackBase;
  void* stackTop;
  allocateStack(&stackBase, &stackTop);
  void* rsp = createStack(0, NULL, idleProcess, stackBase);
  idleProcessPCB.pcb->pid = 0;
  idleProcessPCB.pcb->priority = 0;
  idleProcessPCB.pcb->state = READY;
  idleProcessPCB.pcb->rsp = rsp;

  pcbList.head = NULL;
  pcbList.tail = NULL;
  pcbList.current = NULL;
  pcbList.previous = NULL;

  // pcbList.len = 0;
}

void addPCB(uint32_t pid, void* stack, void* rsp, void* rbp, char* name) {
  PCBnode* node = createPCBNode(pid, 1, READY, rsp, stack, rbp, name);

  if (pcbList.tail == NULL) {
    pcbList.head = node;
    pcbList.tail = node;
  } else {
    pcbList.tail->next = node;
    pcbList.tail = node;
  }
}

void* schedule(void* rsp) {

  if (pcbList.head == NULL) return idleProcessPCB.pcb->rsp;

  if (pcbList.current == NULL) {
    pcbList.current = pcbList.head;
    quantumsLeft = pcbList.current->pcb->priority;
  }
  PCBnode* previousNode = pcbList.current;
  if(pcbList.current->pcb->state == TERMINATED) {
    exitSwitcher();
    while(pcbList.current != previousNode && pcbList.current->pcb->state != READY) {
      pcbList.previous = pcbList.current;
      pcbList.current = pcbList.current->next;
      if(pcbList.current == NULL) {
        pcbList.current = pcbList.head;
      }
    }
  } else {
    pcbList.current->pcb->rsp = rsp;
    pcbList.current->pcb->rbp = (void*)(rsp +8*8); 
  }

  if (quantumsLeft > 0 && pcbList.current->pcb->state == READY) {
    --quantumsLeft;
    return rsp;
  }

 

  pcbList.previous = pcbList.current;
  pcbList.current = pcbList.current->next;
  if (pcbList.current == NULL) pcbList.current = pcbList.head;

  while (pcbList.current != previousNode && pcbList.current->pcb->state != READY) {
    pcbList.previous = pcbList.current;
    pcbList.current = pcbList.current->next;
    if (pcbList.current == NULL) pcbList.current = pcbList.head;
  }

  if (pcbList.current->pcb->state == READY) {
    quantumsLeft = pcbList.current->pcb->priority - 1;
    return pcbList.current->pcb->rsp;
  } else {
    return idleProcessPCB.pcb->rsp;
  }
}

static uint32_t pid = 0;
void* createSwitcher(int argc, char* argv[], void* processRip) {
  void* stackBase;
  void* stackTop;
  allocateStack(&stackBase, &stackTop);
  char** argvStack = stackTop;
  char* arg = stackTop + argc * sizeof(char*);
  int i;
  for(i = 0; i < argc; ++i) {
    argvStack[i] = arg;
    int j = 0;
    for(j; argv[i][j] != 0; j++) {
      arg[j] = argv[i][j];
    }
    arg[j] = 0;
    arg = arg + j + 1;
  }

  void* rsp = createStack(argc, argvStack, processRip, stackBase);
  addPCB(pid++, stackTop, rsp, stackBase, argvStack[0]);
  return rsp;
}

uint32_t createProcess(int argc, char* argv[], void* processRip) {
  createSwitcher(argc, argv, processRip);
  return pid - 1; // Return the pid of the newly created process
}

void* userModProcessInit() {
  char* argv[1] = {"init"};
  void* rsp = createSwitcher(1, argv, userModule);
  pcbList.current = pcbList.head;
  return rsp;
}

void exitSwitcher() {
  

  for (int i = 0; i < pcbList.current->waitingCount; ++i) {
    ProcessControlBlock* pcb = pcbList.current->waitingProcesses[i];
    pcb->state = READY;
    pcb->waitedCode = pcbList.current->pcb->code;
  }

  PCBnode* targetNode = pcbList.current;
  // pcbList.current->pcb->state = EXITED;
  if (targetNode == pcbList.tail) {
    pcbList.head = pcbList.head->next;
    pcbList.current = pcbList.head;
  } else if (targetNode == pcbList.tail) {
      if(pcbList.previous == NULL) {
        1/0;
      }
    // This should never happen and should be removed after testing...
    // I'm using it so I get an exception in case this isn't working as it should.
    //if (pcbList.previous == NULL) 1 / 0;
      pcbList.tail = pcbList.previous;
      pcbList.previous->next = NULL;
      pcbList.current = pcbList.head;
      pcbList.previous = NULL;
    
  } else {

    //targetNode->next = pcbList.current->next;
    freeProcessNode(pcbList.current);
    pcbList.previous->next = targetNode->next;
    pcbList.current = targetNode->next;
  }

  freeProcessNode(targetNode);
}
  void exitProcess(int code) {
    pcbList.current->pcb->state = TERMINATED;
    pcbList.current->pcb->code = code;
  }

  



int waitPid(uint32_t pid) {
  PCBnode* node = pcbList.head;
 
  while (node != NULL && node->pcb->pid <= pid) {
    if (node->pcb->pid == pid) {
      node->waitingProcesses[node->waitingCount++] = pcbList.current->pcb;
      pcbList.current->pcb->state = BLOCKED;
      return pcbList.current->pcb->waitedCode; // Replace for int 0x20 when schedule gets called there.
      // switcherInterruption();
    }
    node = node->next;
  }
  return pcbList.current->pcb->waitedCode; // Return the code of the process that was waited for.
}
