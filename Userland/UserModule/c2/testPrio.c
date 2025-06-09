#include <syscalls.h>
#include <testUtil.h>
#include <shellUtils.h>


#define MINOR_WAIT 1000000
#define WAIT 1000000000

#define TOTAL_PROCESSES 3
#define LOWEST 1
#define MEDIUM 5
#define HIGHEST 9

int64_t prio[TOTAL_PROCESSES] = {LOWEST, MEDIUM, HIGHEST};

void commandTestPriority(int32_t argc, char* argv[argc]) {
  if(argc!=2){
    printf("Usage: %s <0: small wait, 1: long wait>\n", argv[0]);
    sysExit(TOO_MANY_ARGUMENTS);
  }
  if(strcmp(argv[1], "0")!=0 && strcmp(argv[1], "1")!=0){
    printf("Usage: %s <0: small wait, 1: long wait>\n", argv[0]);
    sysExit(ILLEGAL_ARGUMENT);
  }
  int64_t pids[TOTAL_PROCESSES];
  const char* argv2[] = {"commandLoop", argv[1]};
  uint64_t i;

  for (i = 0; i < TOTAL_PROCESSES; i++)
    pids[i] = sysCreateProcess(2, argv2, endless_loop_print);

  bussy_wait(WAIT);
  printf("\nCHANGING PRIORITIES...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    sysChangePriority(pids[i], prio[i]);

  bussy_wait(WAIT);
  printf("\nBLOCKING...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    sysBlockByUser(pids[i]);

  printf("CHANGING PRIORITIES WHILE BLOCKED...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    sysChangePriority(pids[i], MEDIUM);

  printf("UNBLOCKING...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    sysUnblock(pids[i]);

  bussy_wait(WAIT);
  printf("\nKILLING...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    sysKill(pids[i]);
  sysExit(SUCCESS);
}