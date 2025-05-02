#include <syscalls.h>
#include <time.h>

static double freq = 65536 / 3600.0; // interruptions/second
unsigned long getMs() { 
  return sysGetTicks() * 1000 / freq;
}

void sleep(unsigned long ms){
  unsigned long end = ms + getMs();
  while(getMs() < end){
    sysHalt();
  }
}
