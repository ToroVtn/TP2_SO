#ifndef BUDDYMM_H
#define BUDDYMM_H

#include <stdio.h>
#include <stdint.h>

#define NULL (void*)0
#define size_t uint64_t

void buddy_init(void * endOfModules);
void * buddy_malloc(size_t request);
void free(void * ptr);

#endif