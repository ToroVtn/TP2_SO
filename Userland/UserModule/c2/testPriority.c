#include <syscalls.h>
#include <testingUtilities.h>
#include <shellUtils.h>


#define MINOR_WAIT 1000000
#define WAIT 1000000000

#define TOTAL_PROC 3
#define LOW 1
#define MID 5
#define HIGH 9

int64_t prio[TOTAL_PROC] = {LOW, MID, HIGH};

void commandTestPriority(int32_t argc, char* argv[argc]) {
  if(argc!=2){
    printf("Usage: %s <0 for minor wait and 1 for longer wait>\n", argv[0]);
    sysExit(TOO_MANY_ARGUMENTS);
  }
  if(strcmp(argv[1], "0")!=0 && strcmp(argv[1], "1")!=0){
    printf("Usage: %s <0 for minor wait and 1 for longer wait>\n", argv[0]);
    sysExit(ILLEGAL_ARGUMENT);
  }
  int64_t pids[TOTAL_PROC];
  const char* argv2[] = {"commandLoop", argv[1]};
  uint64_t i;

  for (i = 0; i < TOTAL_PROC; i++)
    pids[i] = sysCreateProcess(2, argv2, endless_loop_print);

  bussy_wait(WAIT);
  printf("\nPriorities are changing!!!\n");

  for (i = 0; i < TOTAL_PROC; i++)
    sysSetPriority(pids[i], prio[i]);

  bussy_wait(WAIT);
  printf("\nBlocking!!!\n");

  for (i = 0; i < TOTAL_PROC; i++)
    sysBlockByUser(pids[i]);

  printf("Priorities are changing while blocked!!!\n");

  for (i = 0; i < TOTAL_PROC; i++)
    sysSetPriority(pids[i], MID);

  printf("Unblocking priorities\n");

  for (i = 0; i < TOTAL_PROC; i++)
    sysUnblock(pids[i]);

  bussy_wait(WAIT);
  printf("\nKilling\n");

  for (i = 0; i < TOTAL_PROC; i++)
    sysKill(pids[i]);
  sysExit(SUCCESS);
}