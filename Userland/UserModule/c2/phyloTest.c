#include <shellUtils.h>
#include <utils.h>
#include <syscalls.h>

#define PHYLO_AMOUNT 5
#define PHYLO_MAX_AMOUNT 10
#define PHYLO_MIN_AMOUNT 2

typedef enum { EATING, HUNGRY, THINKING, NOTHING } phylo_state;

typedef struct philosopher{
  phylo_state state;
  int32_t fork_at_index;
  uint32_t pid;
}philosopher;

int32_t phylos_on_table;
int32_t change_phylos_sem;
int32_t print_mutex;
philosopher phylo[PHYLO_MAX_AMOUNT];
int32_t add_rem_value;


int32_t right_fork(int32_t i) {
  return (i + 1) % phylos_on_table;
}

void blockAll() {
  for (int32_t i = 0; i < phylos_on_table; i++) {
    sysWaitSem(change_phylos_sem);
  }
}
void blockAllToInitiate(){
  for (int32_t i = 0; i < PHYLO_AMOUNT; i++) {
    sysWaitSem(change_phylos_sem);
  }
}

void freeAll(){
  for (int32_t i = 0; i < phylos_on_table; i++) {
    sysPostSem(change_phylos_sem);
  }
}

void monitor() {
  sysWaitSem(print_mutex);
  for (int32_t i = 0; i < phylos_on_table; i++) {
    printf("%c ", phylo[i].state ? '.' : 'E');
  }
  printf("\n");
  sysPostSem(print_mutex);
}

void takeForks(int32_t i) {
  phylo[i].state = HUNGRY;
  if (i % 2) {
    sysWaitSem(phylo[right_fork(i)].fork_at_index);
    sysWaitSem(phylo[i].fork_at_index);
  } else {
    sysWaitSem(phylo[i].fork_at_index);
    sysWaitSem(phylo[right_fork(i)].fork_at_index);
  }
}

void leaveForks(int32_t i) {
  sysPostSem(phylo[i].fork_at_index);
  sysPostSem(phylo[right_fork(i)].fork_at_index);
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

void myPhyloProcess(uint64_t argc, char* argv[argc]) {
  int32_t n = strToInt(argv[1]);
  if (n < 0) {
    sysExit(ILLEGAL_ARGUMENT);
  }
  while (1) {
    sysWaitSem(change_phylos_sem);
    think(n);
    takeForks(n);
    eat(n);
    leaveForks(n);
    sysPostSem(change_phylos_sem);
  }
}

int32_t addPhylo(int32_t pos) {
  if (pos < PHYLO_MIN_AMOUNT || pos >= PHYLO_MAX_AMOUNT) return -2;
  blockAll();
  phylo[pos].fork_at_index = sysSemInit(1);
  if (phylo[pos].fork_at_index == -1) {
    printf("Failed creating semaphore\n");
    freeAll();
    return -1;
  }
  phylo[pos].state = THINKING;
  char phylo_num[3];
  uintToBase(pos, phylo_num, 10);
  const char* argvPhylo[] = {"philosopher", phylo_num};
  phylo[pos].pid = sysCreateProcess(sizeof(argvPhylo) / sizeof(argvPhylo[0]), argvPhylo, myPhyloProcess);
  phylos_on_table++;
  freeAll();
  printf("Philosopher number %d has joined the table\n", pos + 1);
  return 0;
}

int32_t remPhylo(int32_t pos) {
  if (pos < PHYLO_MIN_AMOUNT || pos >= phylos_on_table) return -2;
  blockAll();
  if (!sysKill(phylo[pos].pid)){
    printf("Error killing Philosopher %d's process.\n", pos);
    freeAll();
    return -1;
  }
  if (!sysDestroySemaphore(phylo[pos].fork_at_index)) {
    printf("Error destroying Philosopher %d's semaphore.\n", pos);
    freeAll();
    return -1;
  }
  phylos_on_table = pos;
  printf("Philosopher number %d has left the table\n", pos + 1);
  freeAll();
  return 0;
}

void endPhylos() {
  blockAll();
  while (phylos_on_table > 0) {
    int32_t pos = phylos_on_table - 1;
    if (!sysDestroySemaphore(phylo[pos].fork_at_index) || !sysKill(phylo[pos].pid)) {
      printf("Error deleting philosopher %d\n", pos + 1);
      freeAll();
      sysExit(PROCESS_FAILURE);
    }
    printf("Philosopher %d  has left the table\n", pos + 1);
    phylos_on_table--;
  }
  if (!sysDestroySemaphore(change_phylos_sem) || !sysDestroySemaphore(print_mutex)) {
    printf("Error deleting mutex\n");
    sysExit(PROCESS_FAILURE);
  }
  printf("All philosophers have left the table.\n");
}

void commandPhylo(int32_t argc, char* argv[argc]) {
  if (argc != 1) {
    sysExit(TOO_MANY_ARGUMENTS);
  }
  phylos_on_table = 0;
  change_phylos_sem = sysSemInit(PHYLO_AMOUNT);
  print_mutex = sysSemInit(1);
  if (change_phylos_sem == -1 || print_mutex == -1) {
    printf("Error creating mutex\n");
    sysExit(PROCESS_FAILURE);
  }
  blockAllToInitiate();
  for (int32_t i = 0; i < PHYLO_AMOUNT; i++) {
    phylo[i].fork_at_index = sysSemInit(1);
    if (phylo[i].fork_at_index  == -1) {
      printf("Error creating semaphore\n");
      sysExit(PROCESS_FAILURE);
    }
    char philo_num[3];
    uintToBase(i, philo_num, 10);
    const char* argv_phylo[] = {"philosopher", philo_num};
    phylo[i].pid = sysCreateProcess(sizeof(argv_phylo) / sizeof(argv_phylo[0]), argv_phylo, myPhyloProcess);
    phylo[i].state = THINKING;
    phylos_on_table++;
    printf("Philosopher number %d has joined the table\n", i + 1);
  }
  freeAll();

  KeyStruct key;
  while (getKey(&key)) {
    if (key.character == 'a' || key.character == 'A') {
      printf("Adding philosopher %d... \n", phylos_on_table + 1);
      if ((add_rem_value = addPhylo(phylos_on_table)) == -1)
        sysExit(PROCESS_FAILURE);
      else if (add_rem_value == -2){
        printf("Can't add more philosophers\n");
      }
    } else if (key.character == 'r' || key.character == 'R') {
      printf("Removing philosopher %d... \n", phylos_on_table);
      if((add_rem_value = remPhylo(phylos_on_table - 1))==-1)
        sysExit(PROCESS_FAILURE);
      else if (add_rem_value == -2){
        printf("Can't remove more philosophers\n");
      }
    } else if (key.character == 'e' || key.character == 'E') {
      printf("Ending philosophers...\n");
      endPhylos();
      sysExit(SUCCESS);
    }
  }
}