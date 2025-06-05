#ifndef TIMER_H
#define TIMER_H

#include <scheduler.h>

void incTicks();
unsigned long getTicks();
unsigned long getMs();
unsigned long calculateTicks(unsigned long ms);
void sleep(unsigned long ms);

#endif