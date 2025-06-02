#include <memory.h>
#include <scheduler.h>
#include <stdbool.h>
#include <utils.h>


typedef struct PCBNode {
  PCB* pcb;
  struct PCBNode* next;
} PCBNode;

typedef struct {
  PCBNode* head;
  PCBNode* tail;
  PCBNode* current;
  PCBNode* previous;
  int len;
} PCBList;

const char* const stateNames[4] = {"READY", "RUNNING", "BLOCKED", "EXITED"};

extern void* initStack(int argc, char* argv[], void* procRip, void* stackBase);
extern void idleProc();
extern void* userModule;



PCBList pcbList;
PCBNode* idleProcPCBNode;

PCB* createPCB(uint32_t pid, uint8_t priority, State state, void* stack, void* rsp, void* rbp, char* name) {
  PCB* pcb = malloc(sizeof(PCB));
  pcb->pid = pid;
  pcb->priority = priority;
  pcb->state = state;
  pcb->stack = stack;
  pcb->rsp = rsp;
  pcb->rbp = rbp;
  pcb->name = name;
  // pcb->exitCode = 0;
  pcb->waitingPCBCount = 0;

  return pcb;
}

PCBNode* createPCBNode(uint32_t pid, uint8_t priority, State state, void* stack, void* rsp, void* rbp, char* name) {
  PCBNode* node = malloc(sizeof(PCBNode));
  node->next = NULL;
  node->pcb = createPCB(pid, priority, state, stack, rsp, rbp, name);

  return node;
}

// void freePCBNode(PCBNode* node) {
//   free(node->pcb);
//   free(node->stack);
//   free(node);
// }

void addPCB(uint32_t pid, void* stack, void* rsp, void* rbp, char* name) {
  PCBNode* node = createPCBNode(pid, 1, READY, stack, rsp, rbp, name);

  if (pcbList.head == NULL) {
    pcbList.head = node;
    pcbList.head->next = node;
    pcbList.tail = node;
  } else {
    if (pcbList.previous == pcbList.tail) pcbList.previous = node;
    node->next = pcbList.head;
    pcbList.tail->next = node;
    pcbList.tail = node;
  }

  ++pcbList.len;
}

void freeCurrent() {
  if (pcbList.current == NULL || pcbList.head == NULL) return;
  if (pcbList.head == pcbList.tail) return;

  PCBNode* toRemove = pcbList.current;

  pcbList.current = toRemove->next;
  pcbList.previous->next = pcbList.current;
  if (pcbList.head == toRemove) pcbList.head = pcbList.current;
  else if (pcbList.tail == toRemove) pcbList.tail = pcbList.previous;

  free(toRemove->pcb->stack);
  free(toRemove->pcb);
  free(toRemove);

  --pcbList.len;
}

void nextPCB() {
  if (pcbList.head == NULL) return;
  if (pcbList.current == NULL) {
    pcbList.current = pcbList.head;
    pcbList.previous = pcbList.tail;
  } else if (pcbList.current == idleProcPCBNode) {
    pcbList.current = pcbList.previous->next;
  } else {
    pcbList.previous = pcbList.current;
    pcbList.current = pcbList.current->next;
  }
}

void createPCBList() {
  void* stackBase;
  void* stackTop;
  allocateStack(&stackBase, &stackTop);
  void* rsp = initStack(0, NULL, idleProc, stackBase);
  idleProcPCBNode = createPCBNode(-1, 1, READY, stackTop, rsp, rsp, NULL);

  pcbList.head = NULL;
  pcbList.tail = NULL;
  pcbList.current = NULL;
  pcbList.previous = NULL;
  pcbList.len = 0;
}


void* schedule(void* rsp) {
  static int quantumsLeft = 0;

  pcbList.current->pcb->rsp = rsp;
  pcbList.current->pcb->rbp = *(void**)(rsp + 8 * 8);

  if (quantumsLeft > 0 && pcbList.current->pcb->state == RUNNING) {
    --quantumsLeft;
    return rsp;
  } else if (pcbList.current->pcb->state == RUNNING) {
    pcbList.current->pcb->state = READY;
  }

  PCBNode* targetNode = pcbList.current;
  
  nextPCB();
  while (true) {
    if (pcbList.current->pcb->state == READY) {
      pcbList.current->pcb->state = RUNNING;
      quantumsLeft = pcbList.current->pcb->priority - 1;
      return pcbList.current->pcb->rsp;
    } else if (pcbList.current == targetNode) {
      pcbList.current = idleProcPCBNode;
      return idleProcPCBNode->pcb->rsp;
    } else if (pcbList.current->pcb->state == EXITED) {
      
      freeCurrent();
    } else {
      nextPCB();
    }
  }
}
static uint32_t pid = 0;
void* createProc(int argc, char* argv[], void* procRip) {
  void* stackBase;
  void* stackTop;
  allocateStack(&stackBase, &stackTop);
  char** argvStack = stackTop;
  char* arg = stackTop + argc * sizeof(char*);
  for (int i = 0; i < argc; ++i) {
    argvStack[i] = arg;
    int j = strncpy(arg, argv[i], MAX_NAME_LENGTH);
    arg = arg + j + 1;
  }
  void* rsp = initStack(argc, argvStack, procRip, stackBase);
  addPCB(pid++, stackTop, rsp, stackBase, argvStack[0]);
  return rsp;
}

void* initUserModuleProc() {
  char* argv[1] = {"init"};
  void* rsp = createProc(1, argv, userModule);
  nextPCB();
  return rsp;
}

uint32_t initUserProc(int argc, char* argv[], void* procRip) {
  createProc(argc, argv, procRip);
  return pid - 1;
}

void exitProc(int exitCode) {
  pcbList.current->pcb->state = EXITED;
  for (int i = 0; i < pcbList.current->pcb->waitingPCBCount; ++i) {
    PCB* pcb = pcbList.current->pcb->waitingPCBs[i];
    pcb->state = READY;
    pcb->waitedProcCode = exitCode;
  }
}

extern void switcherInterruption();

int waitPid(uint32_t pid) {
  if(pid == pcbList.current->pcb->pid) {
    return pcbList.current->pcb->waitedProcCode; 
  }
  PCBNode* node = pcbList.head;
  
  while (node->pcb->pid <= pid) {
    if (node->pcb->pid == pid && node->pcb->state != EXITED) {
      node->pcb->waitingPCBs[node->pcb->waitingPCBCount++] = pcbList.current->pcb;
      pcbList.current->pcb->state = BLOCKED;
      switcherInterruption(); 
      return pcbList.current->pcb->waitedProcCode;
    }
    node = node->next;
    if(node == pcbList.head) {
     
      break;
    }
  }
  
  return pcbList.current->pcb->waitedProcCode;
}

void convertPCBToUserland(userlandPCB* userlandPcb, PCB* kernelPcb) {
  strcpy(userlandPcb->name, kernelPcb->name);
  userlandPcb->pid = kernelPcb->pid;
  userlandPcb->rsp = kernelPcb->rsp;
  userlandPcb->rbp = kernelPcb->rbp;
  userlandPcb->state = stateNames[kernelPcb->state];
  userlandPcb->priority = kernelPcb->priority;
}
userlandPCB* fetchPCBList(int* len) {
  *len = pcbList.len;
  if (pcbList.head == NULL) return NULL;
  userlandPCB* pcbArray = malloc(sizeof(userlandPCB) * pcbList.len);
  PCBNode* node = pcbList.head;
  for (int i = 0; i < pcbList.len; ++i) {
    convertPCBToUserland(pcbArray + i, node->pcb);
    node = node->next;
  }
  return pcbArray;
}

const PCB* fetchCurrentPCB() {
  return pcbList.current->pcb;
}

void blockProc() {
  pcbList.current->pcb->state = BLOCKED;
  switcherInterruption();
}

void readyProc(const PCB* pcb) {
  ((PCB*)pcb)->state = READY;
}