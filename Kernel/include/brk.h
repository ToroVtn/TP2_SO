#ifndef BRK_H
#define BRK_H

#include <stdint.h>

void buddy_init(void * endOfModules);
void *sbrk(intptr_t increment);
int brk(void * new_break);

#endif