#ifndef MEMORY_H
#define MEMORY_H

#define BUDDY // comment this to change to the other mm implementation (heap4)

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define MAX_STRING_SIZE 1000

#ifdef BUDDY

typedef struct Block {
    struct Block* next;
    uint32_t size;
    bool isFree;
} Block;

void listInit(void* heapStart, Block* freeList[]);

#else

typedef struct Block{
    struct Block * nextFreeBlock;
    size_t blockSize;                    
} Block;

void listInit(void* heapStart, Block* listStart, Block** listEnd, size_t* freeBytes);

#endif

void* malloc(size_t size);
void* globalMalloc(size_t size);
void free(void* ptr);
void globalFree(void * ptr);
void memoryInit(void* heapStart);
void allocateStack(void** rspStart, void** rspEnd);
void* realloc(void* ptr, uint64_t oldSize, uint64_t newSize);

char* getGlobalMemoryState();
char* getProcessMemoryState(uint32_t pid);

#endif