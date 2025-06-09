#include <memoryManager.h>
#include <scheduler.h>

#ifndef BUDDY

#include <stdint.h>
#include <utils.h>

#define BITS_PER_BYTE ((uint64_t) 8)
// Marks the most significant bit in 64 bits as 1, and the rest as 0
#define BLOCK_ALLOCATED_BITMASK (((uint64_t)1) << ((sizeof(uint64_t) * BITS_PER_BYTE) - 1))
#define BLOCK_SIZE_IS_VALID(blockSize) (((blockSize) & BLOCK_ALLOCATED_BITMASK) == 0)
#define BLOCK_IS_ALLOCATED(block) (((block->blockSize) & BLOCK_ALLOCATED_BITMASK) != 0)
#define ALLOCATE_BLOCK(block) ((block->blockSize) |= BLOCK_ALLOCATED_BITMASK)
#define FREE_BLOCK(block) ((block->blockSize) &= ~BLOCK_ALLOCATED_BITMASK)

// 64MB max heap size
#define MAX_MEMORY_AVAILABLE ((1 << 20) * 64)

static Block listStart;
static Block * listEnd = NULL;

static const uint64_t addressByteSize = sizeof(void*);
static const uint64_t structSize = sizeof(Block); // should be 16 bytes

static uint64_t freeBytesRemaining;

// Used so it doesn't split into too small blocks, it's value should be 32 bytes
#define MINIMUM_BLOCK_SIZE_FOR_SPLIT ((uint64_t) (structSize << 1)) 

void internalFreeListInit(void* heapStart, uint32_t heapSize, Block* freeListStart, Block** freeListEnd, uint64_t* bytesAvailable) {
  void * alinedHeapStart = (void*)(((uint64_t)heapStart + addressByteSize - 1) & ~(addressByteSize - 1));
  freeListStart->nextFreeBlock = (Block *) alinedHeapStart;
  freeListStart->blockSize = 0;

  void * alinedHeapEnd = (void*)((uint64_t) (alinedHeapStart + heapSize - structSize));
  *freeListEnd = (Block *) alinedHeapEnd;
  (*freeListEnd)->blockSize = 0;
  (*freeListEnd)->nextFreeBlock = NULL;

  Block * firstFreeBlock = (Block *) alinedHeapStart;
  firstFreeBlock->blockSize = (uint64_t) (alinedHeapEnd - alinedHeapStart);
  firstFreeBlock->nextFreeBlock = *freeListEnd;

  *bytesAvailable = firstFreeBlock->blockSize;

}

void freeListInit(void* heapStart, Block* freeListStart, Block** freeListEnd, uint64_t* bytesAvailable) {
  internalFreeListInit(heapStart, PROCESS_HEAP_SIZE, freeListStart, freeListEnd, bytesAvailable);
}

void memoryInit(void* endOfModules){
  internalFreeListInit(endOfModules, MAX_MEMORY_AVAILABLE, &listStart, &listEnd, &freeBytesRemaining);
}

static void insertBlockIntoFreeList(Block * blockToInsert, Block* freeListStart, Block* freeListEnd){
  Block * blockIterator;
  uint8_t *aux;

  // Iterate through the list until a block is found that has a higher address than the block being inserted. 
  for (blockIterator = freeListStart; blockIterator->nextFreeBlock < blockToInsert; blockIterator = blockIterator->nextFreeBlock) {}

  // Check if block inserted is after blockIterator
  aux = (uint8_t *) blockIterator;

  if ((aux + blockIterator->blockSize) == (uint8_t *)blockToInsert) {
      blockIterator->blockSize += blockToInsert->blockSize;
      blockToInsert = blockIterator;
  }
  // Check if block inserted is right before blockIterator->next
  aux = (uint8_t *)blockToInsert;

  if ((aux + blockToInsert->blockSize) == (uint8_t *) blockIterator->nextFreeBlock) {
      if (blockIterator->nextFreeBlock != freeListEnd) {
          /* Form one big block from the two blocks. */
          blockToInsert->blockSize += blockIterator->nextFreeBlock->blockSize;
          blockToInsert->nextFreeBlock = blockIterator->nextFreeBlock->nextFreeBlock;
      }
      else {
          blockToInsert->nextFreeBlock = freeListEnd;
      }
  }
  else {
      blockToInsert->nextFreeBlock = blockIterator->nextFreeBlock;
  }

  // Check so it doesn't point to itself
  if (blockIterator != blockToInsert) {
      blockIterator->nextFreeBlock = blockToInsert;
  }
}

void* internalMalloc(uint64_t size, Block* freeListStart, Block* freeListEnd, uint64_t* bytesAvailable) {
  Block * block;
  Block * previousBlock;
  Block * newBlockLink;
  void * toReturn = NULL;
  uint64_t alinedRequiredSize = 0;

  if(size > 0){
    // We need to align it since the user can request any size
    alinedRequiredSize = (size + structSize + addressByteSize - 1) & ~(addressByteSize - 1);
  }

  if(BLOCK_SIZE_IS_VALID(alinedRequiredSize) && alinedRequiredSize > 0 && alinedRequiredSize <= (*bytesAvailable)){
    previousBlock = freeListStart;
    block = freeListStart->nextFreeBlock;

    while ((block->blockSize < alinedRequiredSize) && (block->nextFreeBlock != NULL)){
      previousBlock = block;
      block = block->nextFreeBlock;
    }

    // If we reached the end marker, then there's no block of the size needed
    if(block != freeListEnd){
      // The useful memory starts after the block structure
      toReturn = (void *)(previousBlock->nextFreeBlock + 1);

      previousBlock->nextFreeBlock = block->nextFreeBlock;

      if((block->blockSize - alinedRequiredSize) > MINIMUM_BLOCK_SIZE_FOR_SPLIT){
        newBlockLink = (Block *) ((uint8_t *)block + alinedRequiredSize);
        newBlockLink->blockSize = block->blockSize - alinedRequiredSize;
        block->blockSize = alinedRequiredSize;

        insertBlockIntoFreeList(newBlockLink, freeListStart, freeListEnd);
      }

      *bytesAvailable -= block->blockSize;

      ALLOCATE_BLOCK(block);
      block->nextFreeBlock = NULL;
    }
    else{
      return NULL;
    }
  }
  else{
    return NULL;
  }

  return toReturn;
}

void* globalMalloc(uint64_t size) {
  return internalMalloc(size, &listStart, listEnd, &freeBytesRemaining);
}

void* malloc(uint64_t size) {
  PCB* pcb = getCurrentPCB();
  return internalMalloc(size, pcb->freeListStart, pcb->freeListEnd, &(pcb->bytesAvailable));
}

void internalFree(void* ptr, Block* freeListStart, Block* freeListEnd, uint64_t* bytesAvailable) {
  if( ptr == NULL) return;
  // The block structure is before the useful memory
  Block *freeBlock = (Block *) ptr - 1;

  if (BLOCK_IS_ALLOCATED(freeBlock) && freeBlock->nextFreeBlock == NULL) {
      FREE_BLOCK(freeBlock);
      *bytesAvailable += freeBlock->blockSize;
      insertBlockIntoFreeList(((Block *)freeBlock), freeListStart, freeListEnd);
  }
}

void globalFree(void* ptr) {
  internalFree(ptr, &listStart, listEnd, &freeBytesRemaining);
}

void free(void* ptr) {
  if (ptr == NULL) return;
  PCB* pcb = getCurrentPCB();
  if (ptr < pcb->heap || ptr >= pcb->heap + PROCESS_HEAP_SIZE) return;
  internalFree(ptr, pcb->freeListStart, pcb->freeListEnd, &(pcb->bytesAvailable));
}

char* internalGetMemoryState(int32_t heapSize, uint64_t* bytesAvailable) {
  heapSize -= structSize;
  static char* unit = " B ";
  char* toReturn = malloc(MAX_STRING_SIZE);
  if (toReturn == NULL) return NULL;
  int32_t i = strcpy(toReturn, "Total: ");
  i += uintToBase(heapSize, toReturn + i, 10);
  i += strcpy(toReturn + i, unit);
  i += strcpy(toReturn + i, "| Used: ");
  i += uintToBase(heapSize - *bytesAvailable, toReturn + i, 10);
  i += strcpy(toReturn + i, unit);
  i += strcpy(toReturn + i, "| Unused: ");
  i += uintToBase(*bytesAvailable, toReturn + i, 10);
  i += strcpy(toReturn + i, unit);
  toReturn[i] = 0;

  return toReturn;
}

char* getGlobalMemoryState() {
  return internalGetMemoryState(MAX_MEMORY_AVAILABLE, &freeBytesRemaining);
}

char* getProcessMemoryState(uint32_t pid) {
  PCB* pcb = getPCBByPid(pid);
  if (pcb == NULL) return NULL;
  return internalGetMemoryState(PROCESS_HEAP_SIZE, &(pcb->bytesAvailable));
}

#endif