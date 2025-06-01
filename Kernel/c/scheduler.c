#include <memory.h>
#include <scheduler.h>

// The priority based scheduling will work like this:
// - Arranco desde el primer proceso de la pcbList.
// - Ejecuto ese proceso n cantidad de veces seguidas, donde n es la prioridad del proceso.
//   - Esto si se puede quizá convendría que sean 4 quantums seguidos sin context switching.
// - Luego de las n veces paso al siguiente proceso de la lista.

typedef struct PCBNode {
  PCB* pcb;
  PCB* waitingForMe[10]; // This should be of dynamic length
  int wfmLen;
  void* stack;
  struct PCBNode* next;
} PCBNode;

typedef struct {
  PCBNode* head;
  PCBNode* tail;
  PCBNode* current;
  PCBNode* prev;
  // int len;
} PCBList;

extern void* initializeProcessStack(int argc, char* argv[], void* processRip, void* stackStart);
extern void idleProc();
extern void* userModule;
void deleteCurrentProcess();


PCBList pcbList;
PCB idleProcPCB;

PCB* createPCB(uint32_t pid, uint8_t priority, State state, void* rsp, void* rbp, char* name) {
  PCB* pcb = malloc(sizeof(PCB));
  pcb->pid = pid;
  pcb->priority = priority;
  pcb->state = state;
  pcb->rsp = rsp;
  pcb->rbp = rbp;
  pcb->name = name;
  pcb->exitCode = 0;

  return pcb;
}

PCBNode* createPCBNode(uint32_t pid, uint8_t priority, State state, void* stack, void* rsp, void* rbp, char* name) {
  PCBNode* node = malloc(sizeof(PCBNode));
  node->next = NULL;
  node->pcb = createPCB(pid, priority, state, rsp, rbp, name);
  node->wfmLen = 0;
  node->stack = stack;

  return node;
}

void freePCBNode(PCBNode* node) {
  free(node->pcb);
  free(node->stack);
  free(node);
}

void initializePCBList() {
  void* stackStart;
  void* stackEnd;
  stackAlloc(&stackStart, &stackEnd);
  void* rsp = initializeProcessStack(0, NULL, idleProc, stackStart);
  idleProcPCB.pid = 0;
  idleProcPCB.priority = 0;
  idleProcPCB.state = READY;
  idleProcPCB.rsp = rsp;

  pcbList.head = NULL;
  pcbList.tail = NULL;
  pcbList.current = NULL;
  pcbList.prev = NULL;

  // pcbList.len = 0;
}

void addPCB(uint32_t pid, void* stack, void* rsp, void* rbp, char* name) {
  PCBNode* node = createPCBNode(pid, 1, READY, stack, rsp, rbp, name);

  if (pcbList.tail == NULL) {
    pcbList.head = node;
    pcbList.tail = node;
  } else {
    pcbList.tail->next = node;
    pcbList.tail = node;
  }
}

static int quantumsLeft = 0;
void* schedule(void* rsp) {

  ////// Some of this things shouldn't be necessary since adding the userModule process. ////
  if (pcbList.head == NULL) return idleProcPCB.rsp;

  if (pcbList.current == NULL) {
    pcbList.current = pcbList.head;
    quantumsLeft = pcbList.current->pcb->priority;
  }
  // The checks for current != NULL neither (I think).
  ///////////////////////////////////////////////////////////////////////////////////////////

  PCBNode* ogCurrent = pcbList.current;
  if (pcbList.current->pcb->state == EXITED) {
    deleteCurrentProcess();
    while (pcbList.current != ogCurrent && pcbList.current->pcb->state != READY) {
      pcbList.prev = pcbList.current;
      pcbList.current = pcbList.current->next;
      if (pcbList.current == NULL) pcbList.current = pcbList.head;
    }
  } else {
    pcbList.current->pcb->rsp = rsp;
    // rsp is the pointer to the top of the stack after the gpr pushes.
    // Eight addresses down is where rbp was pushed. So `rsp + 8*8` is a
    // pointer to rbp, which is a void*, that's why we cast it to a
    // `void**` and then dereference it to get the actual value of rbp.
    pcbList.current->pcb->rbp = *(void**)(rsp + 8 * 8);

    if (quantumsLeft > 0 && pcbList.current->pcb->state == READY) {
      --quantumsLeft;
      return rsp;
    }

    // We need to do a full loop on the list from current to current because all
    // processes after current could be BLOCKED but some before be READY, or all
    // but current be READY, in which we should continue executing it. Only if
    // no READY process in the *whole* list is found should we default to the
    // idle process.
    do {
      pcbList.prev = pcbList.current;
      pcbList.current = pcbList.current->next;
      if (pcbList.current == NULL) pcbList.current = pcbList.head;
    } while (pcbList.current != ogCurrent && pcbList.current->pcb->state != READY);
  }

  if (pcbList.current->pcb->state == READY) {
    quantumsLeft = pcbList.current->pcb->priority - 1;
    return pcbList.current->pcb->rsp;
  } else {
    // It reached the end of the list without finding any READY process.
    return idleProcPCB.rsp;
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
    int j = 0;
    for (; argv[i][j] != 0; ++j) arg[j] = argv[i][j];
    arg[j] = 0;
    arg = arg + j + 1;
  }
  void* rsp = initializeProcessStack(argc, argvStack, processRip, stackStart);
  addPCB(pid++, stackEnd, rsp, stackStart, argvStack[0]);
  return rsp;
}

void* createUsermModuleProcess() {
  char* argv[1] = {"init"};
  void* rsp = createProcess(1, argv, userModule);
  pcbList.current = pcbList.head;
  return rsp;
}

uint32_t createUserProcess(int argc, char* argv[], void* processRip) {
  createProcess(argc, argv, processRip);
  return pid - 1;
}

void deleteCurrentProcess() {
  // Do something with exitCode?

  for (int i = 0; i < pcbList.current->wfmLen; ++i) {
    PCB* pcb = pcbList.current->waitingForMe[i];
    pcb->state = READY;
    pcb->waitedProcessExitCode = pcbList.current->pcb->exitCode;
  }

  PCBNode* nodeToFree = pcbList.current;

  // This first confition shouldn't really ever be true as the head is the "init"
  // process and it should never exit.
  if (nodeToFree == pcbList.head) {
    pcbList.head = pcbList.head->next;
    pcbList.current = pcbList.head;
  } else if (nodeToFree == pcbList.tail) {
    // This should never happen and should be removed after testing...
    // I'm using it so I get an exception in case this isn't working as it should.
    if (pcbList.prev == NULL) 1 / 0;

    // Note that prev is not NULL as it could only be NULL if the previous condition
    // was true, and then it would not have reached this condition.
    pcbList.tail = pcbList.prev;
    pcbList.tail->next = NULL;
    pcbList.current = pcbList.head;
    pcbList.prev = NULL;
  } else {
    // This should never happen and should be removed after testing...
    // I'm using it so I get an exception in case this isn't working as it should.
    if (pcbList.prev == NULL) 1 / 0;

    // If nodeToFree->next was NULL the previous condition would be true.
    pcbList.prev->next = nodeToFree->next;
    pcbList.current = nodeToFree->next;
  }

  // Freeing here might be wrong as the scheduler will still be running using the
  // freed stack until the schedule() function returns and the context is switched.
  // But meanwhile another process running in another thread (asuming we had multiple
  // threads) might allocate memory and get addresses belonging to the now freed
  // stack, which could technically still be in use. Not sure how I can solve this tho,
  // so for now it will remain like this... (I probably need to find a way to free the
  // stack in asm right before or after switching context)
  freePCBNode(nodeToFree);
}

void exitProcess(int exitCode) {
  pcbList.current->pcb->state = EXITED;
  pcbList.current->pcb->exitCode = exitCode;
}

extern void asdfInterruption();
int waitPid(uint32_t pid) {
  PCBNode* node = pcbList.head;
  // Note pcbList is orded by pid because new nodes are always added at the end and
  // pid is always increasing..
  while (node != NULL && node->pcb->pid <= pid) {
    if (node->pcb->pid == pid) {
      node->waitingForMe[node->wfmLen++] = pcbList.current->pcb;
      pcbList.current->pcb->state = BLOCKED;
      asdfInterruption(); // Replace for int 0x20 when schedule gets called there.
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