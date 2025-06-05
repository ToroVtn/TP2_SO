#include <interruptions.h>
#include <timer.h>
#include <memory.h>

static double freq = 65536 / 3600.0; // interruptions/second
static unsigned long ticks = 0;

typedef struct SleptProcess{
  unsigned long ticksRemaining;
  const PCB* process;
  struct SleptProcess* next;
}SleptProcess;

SleptProcess* first = NULL;

void incTicks() {
  ticks++;
    SleptProcess* currentProcess = first;
    SleptProcess* previousProcess = NULL;

    while (currentProcess != NULL) {
      (currentProcess->ticksRemaining)--;

      if (currentProcess->ticksRemaining == 0) {
          readyProc(currentProcess->process);

          if (previousProcess == NULL) {
              first = currentProcess->next;
          } else {
              previousProcess->next = currentProcess->next;
          }

          SleptProcess* toFree = currentProcess;
          currentProcess = currentProcess->next;
          free(toFree);
      } else {
          previousProcess = currentProcess;
          currentProcess = currentProcess->next;
      }
  }
}

unsigned long getTicks() {
  return ticks;
}

unsigned long getMs() {
  return ticks * 1000 / freq;
}

unsigned long calculateTicks(unsigned long ms) {
  return (unsigned long)(ms * freq / 1000.0);
}

void sleep(unsigned long ms) {
  unsigned long initialTicks = calculateTicks(ms);
  if(initialTicks > 0){
    SleptProcess * aux = malloc(sizeof(*aux));
    aux->ticksRemaining = initialTicks;
    aux->process = fetchCurrentPCB();
    aux->next = first;
    first = aux;
    blockProc();
  }
}