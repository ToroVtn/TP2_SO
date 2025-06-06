#ifndef MEMORY_H
#define MEMORY_H

// #define BUDDY // comment this to change to the other mm implementation

#include <stdint.h>
#include <stdio.h>

#define NULL (void *) 0
#define size_t uint64_t

#ifdef BUDDY
typedef struct Block {
    struct Block* next;
    uint32_t size;
    boolean isFree;
} Block;

#else
typedef struct Block{
    struct Block * nextFreeBlock; /**< The next free block in the list. */
    size_t blockSize;                     /**< The size of the free block. */
} Block;
#endif

void* malloc(uint64_t size);
void free(void* ptr);
void* realloc(void* ptr, uint64_t oldSize, uint64_t newSize);
void memoryInit(void* heapStart);
void allocateStack(void** rspStart, void** rspEnd);

#endif