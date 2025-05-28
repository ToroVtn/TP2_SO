#ifndef BUDDYMM_H
#define BUDDYMM_H

#include <stdint.h>
#include <brk.h>

#define NULL (void*)0
#define size_t uint64_t

void * buddy_malloc(size_t request);
void free(void * ptr);

#endif