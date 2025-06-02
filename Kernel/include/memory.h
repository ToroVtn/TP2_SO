#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stdio.h>

void* malloc(uint64_t size);
void free(void* ptr);
void memoryInit(void* heapStart);
void allocateStack(void** rspStart, void** rspEnd);

#endif