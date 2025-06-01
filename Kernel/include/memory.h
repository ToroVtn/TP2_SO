#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

void* malloc(uint64_t size);
void memoryInit(void * heapStart);
void allocateStack(void** stackBase, void** stackTop);
void free(void* ptr);

#endif