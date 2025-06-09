#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

// comment/uncomment this to change to the other mm implementation
// #define BUDDY

#include <stdbool.h>
#include <stdint.h>

#define NULL (void*)0
#define MAX_STRING_SIZE 1000

#ifdef BUDDY
typedef struct Block {
  struct Block* next;
  uint32_t size;
  bool isFree;
} Block;

void freeListInit(void* heapStart, Block* freeList[]);
#else
typedef struct Block {
  struct Block* nextFreeBlock;
  uint64_t blockSize;
} Block;

void freeListInit(void* heapStart, Block* freeListStart, Block** freeListEnd, uint64_t* bytesAvailable);
#endif

void* globalMalloc(uint64_t size);
void* malloc(uint64_t size);
void globalFree(void* ptr);
void free(void* ptr);
void memoryInit(void* heapStart);
// must be freed upon finishing it's use
char* getGlobalMemoryState();
char* getProcessMemoryState(uint32_t pid);

// bellow are the functions in memory.c

void stackAlloc(void** stackStart, void** stackEnd);
void* shittyRealloc(void* ptr, uint64_t oldSize, uint64_t newSize);

#endif