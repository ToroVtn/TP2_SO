#include <memory.h>
#include <scheduler.h>
#include <stdbool.h>
#include <utils.h>
#include <pipes.h>
#include <videoDriver.h>


#define IDLE_PID -1

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

const char* const stateNames[6] = {"READY", "RUNNING", "BLOCKED", "EXITED", "W-EXIT", "USER_BLOCKED"};


extern void* initStack(int argc, char* argv[], void* procRip, void* stackBase);
extern void idleProc();
extern void* userModule;
extern void switcherInterruption();

PCB* getPCB(uint32_t pid);
void exitProcessByPCB(PCB* pcb, int exitCode); 

PCBList pcbList;
PCBNode* idleProcPCBNode;
PCB* processInForeground;

PCB* createPCB(uint32_t pid, uint8_t priority, State state, void* stack, void* rsp, void* rbp, char* name) {
  PCB* pcb = globalMalloc(sizeof(PCB));
  pcb->pid = pid;
  pcb->priority = priority;
  pcb->state = state;
  pcb->stack = stack;
  pcb->rsp = rsp;
  pcb->rbp = rbp;
  pcb->name = name;
  pcb->waitingPCBCount = 0;
  pcb->pipes.read = STDIN;
  pcb->pipes.write = STDOUT;
  pcb->pipes.err = STDERR;

  return pcb;
}

PCBNode* createPCBNode(
    uint32_t pid, uint8_t priority, State state, void* stack, void* rsp, void* rbp, char* name, ProcessPipes pipes
) {
  PCBNode* node = globalMalloc(sizeof(PCBNode));
  if (node == NULL) {
    return NULL;
  }
  node->next = NULL;

  PCB* pcb = globalMalloc(sizeof(PCB));
  if (pcb == NULL) {
    globalFree(node);
    return NULL;
  }
  pcb->pid = pid;
  pcb->priority = priority;
  pcb->state = state;
  pcb->stack = stack;
  pcb->rsp = rsp;
  pcb->rbp = rbp;
  pcb->name = name;
  pcb->parent = pcbList.current->pcb;
  pcb->waitingPCBCount = 0;
  pcb->pipes = pipes;
  if ((int32_t)pid != IDLE_PID) {
    pcb->heap = globalMalloc(PROCESS_HEAP_SIZE);
    if (pcb->heap == NULL) {
      globalFree(node);
      globalFree(pcb);
      return NULL;
    }
#ifdef BUDDY
    listInit(pcb->heap, pcb->freeList);
#else
    pcb->listStart = globalMalloc(sizeof(Block));
    if (pcb->listStart == NULL) {
      globalFree(node);
      globalFree(pcb);
      globalFree(pcb->heap);
      return NULL;
    }
    listInit(pcb->heap, pcb->listStart, &(pcb->listEnd), &(pcb->bytesAvailable));
#endif
  } else pcb->heap = NULL;
  pcb->heapFreed = false;
  node->pcb = pcb;
  
  return node;
}



bool addPCB(uint32_t pid, void* stack, void* rsp, void* rbp, char* name, ProcessPipes pipes) {
  PCBNode* node = createPCBNode(pid, 1, READY, stack, rsp, rbp, name, pipes);

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
  return true;
}

void freeCurrent() {
  if (pcbList.current == NULL || pcbList.head == NULL){
    return;
  } 
  if (pcbList.head == pcbList.tail) {
    return;
  }

  PCBNode* toRemove = pcbList.current;

  pcbList.current = toRemove->next;
  pcbList.previous->next = pcbList.current;
  if (pcbList.head == toRemove){
    pcbList.head = pcbList.current;
  }
  else if (pcbList.tail == toRemove) {
    pcbList.tail = pcbList.previous;
  }
  globalFree(toRemove->pcb->stack);
  globalFree(toRemove->pcb);
  globalFree(toRemove);

  --pcbList.len;
}

void nextPCB() {
  if (pcbList.head == NULL) {
    return;
  }
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
  ProcessPipes pipes = {.write = STDOUT, .read = STDIN, .err = STDERR};
  idleProcPCBNode = createPCBNode(-1, 1, READY, stackTop, rsp, rsp, "idle", pipes);
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
  if(pcbList.current == idleProcPCBNode) {
    nextPCB();
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
void* createProc(int argc, char* argv[], void* procRip, ProcessPipes pipes) {
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
  if (!addPCB(pid++, stackTop, rsp, stackBase, argvStack[0], pipes)) return NULL;
  return rsp;
}

void* initUserModuleProc() {
  const char* argv[1] = {"init"};
  ProcessPipes pipes = {.write = STDOUT, .read = STDIN, .err = STDERR};
  void* rsp = createProc(1, argv, userModule, pipes);
  if (rsp == NULL) return NULL;
  nextPCB();
  processInForeground = pcbList.current->pcb;
  pcbList.current->pcb->parent = NULL;
 
  return rsp;
}

int32_t initUserProcWithPipeSwap(int32_t argc, const char* argv[], void* processRip, ProcessPipes pipes) {
  if (createProc(argc, argv, processRip, pipes) == NULL) return -1;
  return pid - 1;
}

uint32_t initUserProc(int argc, char* argv[], void* procRip) {
  ProcessPipes pipes = {.write = STDOUT, .read = STDIN, .err = STDERR};
  if (createProc(argc, argv, procRip, pipes) == NULL) return -1;
  return pid - 1;
}


void exitProcessByPCB(PCB* pcb, int32_t exitCode) {
  if (pcb->state == EXITED) return;
  if (pcb->state == BLOCKED) {
    pcb->state = STANDBY_FOR_EXIT;
    globalFree(pcb->heap);
    pcb->heapFreed = true;
    if (pcb->pid == pcbList.current->pcb->pid) switcherInterruption();
    return;
  }

  pcb->state = EXITED;
  if (pcb->pid == processInForeground->pid) processInForeground = pcb->parent;
  for (int32_t i = 0; i < pcb->waitingPCBCount; ++i) {
    PCB* pcb2 = pcb->waitingPCBs[i];
    if (pcb2->state == BLOCKED) {
      pcb2->state = READY;
      pcb2->waitedProcCode = exitCode;
    } else if (pcb2->state == STANDBY_FOR_EXIT) {
      exitProcessByPCB(pcb2, KILL_CODE);
    }
  }
  if (pcb->pid == pcbList.current->pcb->pid) switcherInterruption();
}

void exitProc(int exitCode) {
  exitProcessByPCB(pcbList.current->pcb, exitCode);
  switcherInterruption(); // Switch to the next process
}


PCB* getPCB(uint32_t pid) { // gets the pcb by using pid
  PCBNode* node = pcbList.head;
  do {
    if (node->pcb->pid == pid) {
      return node->pcb;
    }
    node = node->next;
  } while (node->pcb->pid <= pid && node != pcbList.head);
  return NULL;
}

int waitPid(uint32_t pid) {
  if (pid == pcbList.current->pcb->pid) return pcbList.current->pcb->waitedProcCode;

  PCB* pcb = getPCB(pid);
  if (pcb == NULL || pcb->state == EXITED) return pcbList.current->pcb->waitedProcCode;
  pcb->waitingPCBs[pcb->waitingPCBCount++] = pcbList.current->pcb;
  pcbList.current->pcb->state = BLOCKED;
  if (pcbList.current->pcb->pid == processInForeground->pid) processInForeground = pcb;
  switcherInterruption();
  return pcbList.current->pcb->waitedProcCode;
}


void convertPCBToUserland(userlandPCB* userlandPcb, PCB* kernelPcb) {
  strcpy(userlandPcb->name, kernelPcb->name);
  userlandPcb->pid = kernelPcb->pid;
  userlandPcb->rsp = kernelPcb->rsp;
  userlandPcb->rbp = kernelPcb->rbp;
  userlandPcb->state = stateNames[kernelPcb->state];
  userlandPcb->priority = kernelPcb->priority;
  userlandPcb->location = (kernelPcb->pid == processInForeground->pid) ? "foreground" : "background";
}
userlandPCB* fetchPCBList(int* len) {
  *len = pcbList.len;
  if (pcbList.head == NULL) return NULL;
  userlandPCB* pcbArray = globalMalloc(sizeof(userlandPCB) * pcbList.len);
  if(pcbArray == NULL) return NULL;
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

uint32_t getpid() {
  return pcbList.current->pcb->pid;
}

bool kill(uint32_t pid) {
  if (pid == 0) return false;
  PCB* pcb = getPCB(pid);
  if (pcb == NULL || pcb->state == EXITED || pcb->state == STANDBY_FOR_EXIT) return false;
  exitProcessByPCB(pcb, KILL_CODE);
  return true;
}

void killCurrentProcess() {
  kill(pcbList.current->pcb->pid);
}

void killForegroundProc() {
  if (processInForeground == 0) return;
  exitProcessByPCB(processInForeground, KILL_CODE);
  switcherInterruption(); // Switch to the next process
}

bool setPriority(uint32_t pid, uint8_t newPriority) {
  if (newPriority <= 0 || newPriority > 9) return false;
  PCB* pcb = getPCB(pid);
  if (pcb == NULL) return false;
  pcb->priority = newPriority;
  return true; 
}



bool block(uint32_t pid) {
  PCB* pcb = getPCB(pid);
  if (pcb != NULL && (pcb->state == READY || pcb->state == RUNNING)) {
    pcb->state = USER_BLOCKED;
    if (pcb->pid == pcbList.current->pcb->pid) switcherInterruption();
    return true;
  }
  return false;
}

bool unBlock(uint32_t pid) {
  PCB* pcb = getPCB(pid);
  if (pcb != NULL && pcb->state == USER_BLOCKED) {
    pcb->state = READY;
    return true;
  }
  return false;
}


void changePipeRead(int32_t pipe) {
  pcbList.current->pcb->pipes.read = pipe;
}

void changePipeWrite(int32_t pipe) {
  pcbList.current->pcb->pipes.write = pipe;
}

ProcessPipes fetchPipes() {
  return pcbList.current->pcb->pipes;
}

int64_t read(int32_t pipeId, char* buf, int32_t len) {
  if (pipeId == STDIN)  {
    return readStdin(buf, len);
  }
  return readFromPipe(pipeId, buf, len);
}

int64_t write(int32_t pipeId, const char* buf, int32_t len) {
  if (pipeId == STDOUT) {
    printNextBuf(buf, len);
    return len;
  }
  return writeToPipe(pipeId, buf, len);
}

void yield() {
  pcbList.current->pcb->state = READY;
  switcherInterruption();
}