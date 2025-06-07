#ifndef MEMORY_H
#define MEMORY_H

// #define BUDDY // comment this to change to the other mm implementation

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef BUDDY

typedef struct Block {
    struct Block* next;
    uint32_t size;
    bool isFree;
} Block;

void listInit(void* heapStart, Block* freeList[]);

#else

typedef struct Block{
    struct Block * nextFreeBlock; /**< The next free block in the list. */
    size_t blockSize;                     /**< The size of the free block. */
} Block;

void listInit(void* heapStart, Block* listStart, Block** listEnd, size_t* freeBytes);

#endif

void* malloc(size_t size);
void* globalMalloc(size_t size);
void free(void* ptr);
void globalF
void* realloc(void* ptr, uint64_t oldSize, uint64_t newSize);
void memoryInit(void* heapStart);
void allocateStack(void** rspStart, void** rspEnd);

#endif