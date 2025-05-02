#include <timer.h>
#include <interruptions.h>

static double freq = 65536 / 3600.0; // interruptions/second
static unsigned long ticks = 0;
void timerTick() { ticks++; }
unsigned long getTicks() { return ticks; }
unsigned long getMs() { return ticks * 1000 / freq; }
void sleep(unsigned long ms){
unsigned long end = ms + getMs();
    while(getMs() < end){
        haltTillNextInterruption();
    }
}
