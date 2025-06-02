#include <memory.h>
#include <scheduler.h>
#include <stdbool.h>
#include <utils.h>

// El priority based scheduling funciona así:
// - Arranco desde el primer proceso de la pcbList.
// - Ejecuto ese proceso n cantidad de veces seguidas, donde n es la prioridad del proceso.
//   - Esto si se puede quizá convendría que sean 4 quantums seguidos sin context switching.
// - Luego de las n veces paso al siguiente proceso de la lista.

typedef struct PCBNode {
  PCB* pcb;
  struct PCBNode* next;
} PCBNode;

typedef struct {
  PCBNode* head;
  PCBNode* tail;
  PCBNode* current;
  PCBNode* prev;
  int len;
} PCBList;

const char* const StateStrings[4] = {"READY", "RUNNING", "BLOCKED", "EXITED"};

extern void* initializeProcessStack(int argc, char* argv[], void* processRip, void* stackStart);
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
  pcb->wfmLen = 0;

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
    if (pcbList.prev == pcbList.tail) pcbList.prev = node;
    node->next = pcbList.head;
    pcbList.tail->next = node;
    pcbList.tail = node;
  }

  ++pcbList.len;
}

void freeCurrentProcess() {
  if (pcbList.current == NULL || pcbList.head == NULL) return;
  if (pcbList.head == pcbList.tail) return;

  PCBNode* toRemove = pcbList.current;

  pcbList.current = toRemove->next;
  pcbList.prev->next = pcbList.current;
  if (pcbList.head == toRemove) pcbList.head = pcbList.current;
  else if (pcbList.tail == toRemove) pcbList.head = pcbList.prev;

  free(toRemove->pcb->stack);
  free(toRemove->pcb);
  free(toRemove);

  --pcbList.len;
}

void nextPCB() {
  if (pcbList.head == NULL) return;
  if (pcbList.current == NULL) {
    pcbList.current = pcbList.head;
    pcbList.prev = pcbList.tail;
  } else if (pcbList.current == idleProcPCBNode) {
    pcbList.current = pcbList.prev->next;
  } else {
    pcbList.prev = pcbList.current;
    pcbList.current = pcbList.current->next;
  }
}

void initializePCBList() {
  void* stackStart;
  void* stackEnd;
  stackAlloc(&stackStart, &stackEnd);
  void* rsp = initializeProcessStack(0, NULL, idleProc, stackStart);
  idleProcPCBNode = createPCBNode(-1, 1, READY, stackEnd, rsp, rsp, NULL);

  pcbList.head = NULL;
  pcbList.tail = NULL;
  pcbList.current = NULL;
  pcbList.prev = NULL;
  pcbList.len = 0;
}

// Should only be called after creating userModule process, so head and current won't be NULL.
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

  PCBNode* ogCurrent = pcbList.current;
  // We need to always go to next process first so we
  // 1) don't repeat current if it's READY, and
  // 2) don't free current process while we are inside its own stack.
  nextPCB();
  while (true) {
    if (pcbList.current->pcb->state == READY) {
      pcbList.current->pcb->state = RUNNING;
      quantumsLeft = pcbList.current->pcb->priority - 1;
      return pcbList.current->pcb->rsp;
    } else if (pcbList.current == ogCurrent) {
      // Note: this check also prevents the case where a process exits and all other processes
      // are blocked, which would otherwise also lead to freeing the current stack.
      pcbList.current = idleProcPCBNode;
      return idleProcPCBNode->pcb->rsp;
    } else if (pcbList.current->pcb->state == EXITED) {
      // Note that freeCurrentProcess changes current pcb so we need to avoid running
      // nextPCB() in this branch.
      freeCurrentProcess();
    } else {
      nextPCB();
    }
  }
}

/*
Process arguments (argv) will be passed as a pointer to a userland local
array (aka will be on the stack of the parent function/process), so it
could be deleted before the child process finishes using it. That's why
we need to copy it into the current process' stack.
I decided to add argv at the end/top of the stack, and the strings themselves
just bellow it. This way the process can use the stack from the bottom without
ever knowing about it (unless it reaches the end of the stack which should
never happen).

argc = 3

         |- stackEnd, argvStack
         v
0x503d8  00 00 00 00 00 05 03 f0 <-- argvStack[0]: char* to "Arg 1"
0x503e0  00 00 00 00 00 05 03 f6 <-- argvStack[1]: char* to "Arg 2"
0x503e8  00 00 00 00 00 05 03 fc <-- argvStack[2]: char* to "Arg argc"

         |- stackEnd + argc * sizeof(char*)
         |- argvStack[0]
         |                 |- 0x503f6
         |                 |- argvStack[1]
         v                 v
0x503f0  41 72 67 20 31 00 41 72 <-- "Arg 1", "Ar"

                     |- 0x503fc
                     |- argvStack[2]
                     v
0x503f8  67 20 32 00 41 72 67 20 <-- "g 2", "Arg "
0x50400  33 00 00 00 00 00 00 00 <-- "3"
0x50408  00 00 00 00 00 00 00 00
0x50410  00 00 00 00 00 00 00 00
0x50418  00 00 00 00 00 00 00 00
   .
   . stackStart
   .     v
0xXXXXX  00 00 00 00 00 00 00 00
 */
static uint32_t pid = 0;
void* createProcess(int argc, char* argv[], void* processRip) {
  void* stackStart;
  void* stackEnd;
  stackAlloc(&stackStart, &stackEnd);
  char** argvStack = stackEnd;
  char* arg = stackEnd + argc * sizeof(char*);
  for (int i = 0; i < argc; ++i) {
    argvStack[i] = arg;
    int j = strncpy(arg, argv[i], MAX_NAME_LENGTH);
    arg = arg + j + 1;
  }
  void* rsp = initializeProcessStack(argc, argvStack, processRip, stackStart);
  addPCB(pid++, stackEnd, rsp, stackStart, argvStack[0]);
  return rsp;
}

void* createUserModuleProcess() {
  char* argv[1] = {"init"};
  void* rsp = createProcess(1, argv, userModule);
  nextPCB();
  return rsp;
}

uint32_t createUserProcess(int argc, char* argv[], void* processRip) {
  createProcess(argc, argv, processRip);
  return pid - 1;
}

void exitCurrentProcess(int exitCode) {
  pcbList.current->pcb->state = EXITED;
  for (int i = 0; i < pcbList.current->pcb->wfmLen; ++i) {
    PCB* pcb = pcbList.current->pcb->waitingForMe[i];
    pcb->state = READY;
    pcb->waitedProcessExitCode = exitCode;
  }
}

extern void asdfInterruption();
int waitPid(uint32_t pid) {
  PCBNode* node = pcbList.head;
  // Note pcbList is orded by pid because new nodes are always added at the end and
  // pid is always increasing..
  while (node->pcb->pid <= pid) {
    if (node->pcb->pid == pid) {
      node->pcb->waitingForMe[node->pcb->wfmLen++] = pcbList.current->pcb;
      pcbList.current->pcb->state = BLOCKED;
      asdfInterruption(); // Replace for int 0x20 when schedule gets called there.
      return pcbList.current->pcb->waitedProcessExitCode;
    }
    node = node->next;
  }
  // Este return value no tiene sentido si el proceso se corrió en el background. Igual
  // casi seguro que tengo que tener en cuenta padres e hijos así que probablemente el
  // exit solo me mande el pcb del proceso hijo a una lista en el padre, y recién una
  // vez que el padre termina se libera todo lo relacionado a los hijos que terminaron.
  // Y ahí sí podría conseguir la referencia al hijo aunque ya haya terminado.
  // If no process with the specified pid is found then current process is not blocked.
  return pcbList.current->pcb->waitedProcessExitCode;
}

void copyPCBToPCBForUserland(PCBForUserland* userlandPcb, PCB* kernelPcb) {
  strcpy(userlandPcb->name, kernelPcb->name);
  userlandPcb->pid = kernelPcb->pid;
  userlandPcb->rsp = kernelPcb->rsp;
  userlandPcb->rbp = kernelPcb->rbp;
  userlandPcb->state = StateStrings[kernelPcb->state];
  userlandPcb->priority = kernelPcb->priority;
}
PCBForUserland* getPCBList(int* len) {
  *len = pcbList.len;
  if (pcbList.head == NULL) return NULL;
  PCBForUserland* pcbArray = malloc(sizeof(PCBForUserland) * pcbList.len);
  PCBNode* node = pcbList.head;
  for (int i = 0; i < pcbList.len; ++i) {
    copyPCBToPCBForUserland(pcbArray + i, node->pcb);
    node = node->next;
  }
  return pcbArray;
}