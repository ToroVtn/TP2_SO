#ifndef MEMORY_H
#define MEMORY_H

// #define BUDDY // comment this to change to the other mm implementation

#include <stdint.h>

#define NULL (void *) 0
#define size_t uint64_t

void* malloc(uint64_t size);
void free(void* ptr);
void memoryInit(void* heapStart);

#endif