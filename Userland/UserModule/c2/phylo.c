#include <shellUtils.h>
#include <utils.h>
#include <syscalls.h>

#define QUANTITY_PHYLO 5
#define PHYLO_MAX 10
#define PHYLO_MIN 2

typedef enum { EATING, HUNGRY, THINKING, NOTHING } state;

typedef struct philosopher{
  state state;
  int32_t forkAtIndex;
  uint32_t pid;
}philosopher;

int32_t phylosEating;
int32_t phyloSem;
int32_t printMutex;
philosopher phylo[PHYLO_MAX];
int32_t opStatus;

int32_t opMutex;

void allWait() {
  // Just acquire a single mutex for operations instead of blocking all philosophers
  sysWaitSem(opMutex);
}

// Replace the inefficient freeAll function
void freeAll() {
  // Just release the operation mutex
  sysPostSem(opMutex);
}

int32_t rightFork(int32_t i) {
  return (i + 1) % phylosEating;
}

/* void allWait() {
  for (int32_t i = 0; i < phylosEating; i++) {
    sysWaitSem(phyloSem);
  }
} */
void allWaitForInit(){
  for (int32_t i = 0; i < QUANTITY_PHYLO; i++) {
    sysWaitSem(phyloSem);
  }
}

/* void freeAll(){
  for (int32_t i = 0; i < phylosEating; i++) {
    sysPostSem(phyloSem);
  }
}
 */
void monitor() {
  sysWaitSem(printMutex);
  for (int32_t i = 0; i < phylosEating; i++) {
    printf("%c ", phylo[i].state ? '.' : 'E');
  }
  printf("\n");
  sysPostSem(printMutex);
}

void grabForks(int32_t i) {
  phylo[i].state = HUNGRY;
  if (i % 2) {
    sysWaitSem(phylo[rightFork(i)].forkAtIndex);
    sysWaitSem(phylo[i].forkAtIndex);
  } else {
    sysWaitSem(phylo[i].forkAtIndex);
    sysWaitSem(phylo[rightFork(i)].forkAtIndex);
  }
}

void fropForks(int32_t i) {
  sysPostSem(phylo[i].forkAtIndex);
  sysPostSem(phylo[rightFork(i)].forkAtIndex);
  phylo[i].state = NOTHING;
}

void think(int32_t i) {
  phylo[i].state = THINKING;
  sysSleep(randBetween(600, 800));
}
void eat(int32_t i) {
  phylo[i].state = EATING;
  sysSleep(randBetween(600, 800));
  monitor();
}

void phyloLoop(uint64_t argc, char* argv[argc]) {
  int32_t n = strToInt(argv[1]);
  if (n < 0) {
    sysExit(ILLEGAL_ARGUMENT);
  }
  while (1) {
    sysWaitSem(phyloSem);
    think(n);
    grabForks(n);
    eat(n);
    fropForks(n);
    sysPostSem(phyloSem);
  }
}

int32_t addPhylo(int32_t pos) {
  if (pos < PHYLO_MIN || pos >= PHYLO_MAX) return -2;
  
  // Use the operation mutex
  sysWaitSem(opMutex);
  
  // Create the fork semaphore
  phylo[pos].forkAtIndex = sysSemInit(1);
  if (phylo[pos].forkAtIndex == -1) {
    printf("Failed creating semaphore\n");
    sysPostSem(opMutex);
    return -1;
  }
  
  // Initialize the philosopher
  phylo[pos].state = THINKING;
  char phyloNum[3];
  uintToBase(pos, phyloNum, 10);
  const char* argvPhylo[] = {"philosopher", phyloNum};
  phylo[pos].pid = sysCreateProcess(sizeof(argvPhylo) / sizeof(argvPhylo[0]), argvPhylo, phyloLoop);
  phylosEating++;
  
  // Release the operation mutex
  sysPostSem(opMutex);
  
  printf("Philosopher number %d has joined the table\n", pos + 1);
  return 0;
}
int32_t removePhylo(int32_t pos) {
  if (pos < PHYLO_MIN || pos >= phylosEating) return -2;
  allWait();
  if (!sysKill(phylo[pos].pid)){
    printf("Error killing Philosopher %d's process.\n", pos);
    freeAll();
    return -1;
  }
  if (!sysDestroySemaphore(phylo[pos].forkAtIndex)) {
    printf("Error destroying Philosopher %d's semaphore.\n", pos);
    freeAll();
    return -1;
  }
  phylosEating = pos;
  printf("Philosopher number %d has left the table\n", pos + 1);
  freeAll();
  return 0;
}

void endPhylos() {
  allWait();
  while (phylosEating > 0) {
    int32_t pos = phylosEating - 1;
    if (!sysDestroySemaphore(phylo[pos].forkAtIndex) || !sysKill(phylo[pos].pid)) {
      printf("Error deleting philosopher %d\n", pos + 1);
      freeAll();
      sysExit(PROCESS_FAILURE);
    }
    printf("Philosopher %d  has left the table\n", pos + 1);
    phylosEating--;
  }
  if (!sysDestroySemaphore(phyloSem) || !sysDestroySemaphore(printMutex)) {
    printf("Error deleting mutex\n");
    sysExit(PROCESS_FAILURE);
  }
  printf("All philosophers have left the table.\n");
}

// FIRST: Remove allWaitForInit completely
// void allWaitForInit() { ... } <- DELETE THIS FUNCTION

// SECOND: Update commandPhylo to fix deadlocks and CPU usage
void commandPhylo(int32_t argc, char* argv[argc]) {
  if (argc != 1) {
    sysExit(TOO_MANY_ARGUMENTS);
  }
  
  phylosEating = 0;
  
  // Create the operation mutex first
  opMutex = sysSemInit(1);
  if (opMutex == -1) {
    printf("Error creating operation mutex\n");
    sysExit(PROCESS_FAILURE);
  }
  
  // Create the print mutex
  printMutex = sysSemInit(1);
  if (printMutex == -1) {
    printf("Error creating print mutex\n");
    sysExit(PROCESS_FAILURE);
  }
  
  // Create all fork semaphores first
  for (int32_t i = 0; i < QUANTITY_PHYLO; i++) {
    phylo[i].forkAtIndex = sysSemInit(1);
    if (phylo[i].forkAtIndex == -1) {
      printf("Error creating fork semaphore\n");
      sysExit(PROCESS_FAILURE);
    }
    
    phylo[i].state = THINKING;
    phylosEating++;
    printf("Setting up philosopher %d\n", i + 1);
  }
  
  // Create the phyloSem
  phyloSem = sysSemInit(QUANTITY_PHYLO);
  if (phyloSem == -1) {
    printf("Error creating change semaphore\n");
    sysExit(PROCESS_FAILURE);
  }
  
  // Start the philosopher processes
  for (int32_t i = 0; i < QUANTITY_PHYLO; i++) {
    char philo_num[3];
    uintToBase(i, philo_num, 10);
    const char* argv_phylo[] = {"philosopher", philo_num};
    phylo[i].pid = sysCreateProcess(sizeof(argv_phylo) / sizeof(argv_phylo[0]), argv_phylo, phyloLoop);
    printf("Philosopher number %d has joined the table\n", i + 1);
  }
  
  // 3. Improve the keyboard handling loop
  printf("\nCommands:\n");
  printf("  a - Add philosopher\n");
  printf("  r - Remove philosopher\n");
  printf("  e - Exit\n\n");
  
  KeyStruct key;
  while (1) {
    
    // Poll for keyboard input
    if (!getKey(&key)) {
      continue; // No key pressed, try again
    }
    
    // Process key immediately
    switch(key.character) {
      case 'a':
      case 'A':
        printf("\nAdding philosopher %d... \n", phylosEating + 1);
        if ((opStatus = addPhylo(phylosEating)) == -1)
          sysExit(PROCESS_FAILURE);
        else if (opStatus == -2)
          printf("Can't add more philosophers\n");
        break;
        
      case 'r':
      case 'R':
        printf("\nRemoving philosopher %d... \n", phylosEating);
        if((opStatus = removePhylo(phylosEating - 1)) == -1)
          sysExit(PROCESS_FAILURE);
        else if (opStatus == -2)
          printf("Can't remove more philosophers\n");
        break;
        
      case 'e':
      case 'E':
        printf("\nEnding philosophers...\n");
        endPhylos();
        sysExit(SUCCESS);
        break;
    }
  }
}