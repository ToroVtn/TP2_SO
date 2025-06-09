#include <memory.h>
#include <scheduler.h>

#ifdef BUDDY

#include <stdint.h>
#include <utils.h>

typedef enum { LEFT = 'L', RIGHT = 'R' } blockAlignment;

#define ORDER_COUNT 27
// 2^(ORDER_COUNT-1) bytes
#define MAX_MEMORY_AVAILABLE (1 << (ORDER_COUNT - 1))

static const uint64_t addressByteSize = sizeof(void*);

Block* freeList[ORDER_COUNT];
void* iniAddress;

void internalFreeListInit(int32_t orderCount, void* heapStart, uint32_t heapSize, Block* freeList[]) {
  for (int32_t i = 0; i < orderCount; i++) {
    freeList[i] = NULL;
  }
  Block* initialBlock = (Block*)(((uint64_t)heapStart + addressByteSize - 1) & ~(addressByteSize - 1));
  initialBlock->size = heapSize;
  initialBlock->isFree = true;
  initialBlock->next = NULL;
  freeList[orderCount - 1] = initialBlock;
}

void listInit(void* heapStart, Block* freeList[]) {
  internalFreeListInit(PROCESS_HEAP_ORDER_COUNT, heapStart, PROCESS_HEAP_SIZE, freeList);
}

void memoryInit(void* endOfModules) {
  iniAddress = endOfModules;
  internalFreeListInit(ORDER_COUNT, endOfModules, MAX_MEMORY_AVAILABLE, freeList);
}

static int32_t getOrder(uint32_t size) {
  uint32_t order = 0;
  uint32_t iniBlockSize = 1;
  while (iniBlockSize < size) {
    iniBlockSize <<= 1;
    order++;
  }
  return order;
}

static Block* splitBlock(Block* block) {
  uint32_t newBlockSize = block->size / 2;
  block->size = newBlockSize;
  Block* buddy = (Block*)((char*)block + newBlockSize);
  buddy->size = newBlockSize;
  buddy->isFree = true;
  return buddy;
}

void* internalMalloc(uint64_t size, int32_t orderCount, Block* freeList[]) {
  int32_t order = getOrder(size + sizeof(Block));
  for (int32_t currentOrder = order; currentOrder < orderCount; currentOrder++) {
    if (freeList[currentOrder] != NULL && freeList[currentOrder]->isFree) {
      Block* block = freeList[currentOrder];
      freeList[currentOrder] = block->next;
      while (order < currentOrder) {
        currentOrder--;
        Block* buddy = splitBlock(block);
        buddy->next = freeList[currentOrder];
        freeList[currentOrder] = buddy;
      }
      block->isFree = false;
      return (void*)(block + 1);
    }
  }
  return NULL;
}

void* globalMalloc(uint64_t size) {
  return internalMalloc(size, ORDER_COUNT, freeList);
}

void* malloc(uint64_t size) {
  PCB* pcb = fetchCurrentPCB();
  return internalMalloc(size, PROCESS_HEAP_ORDER_COUNT, pcb->freeList);
}

static void removeFromFreeList(Block* toRemove, uint32_t order, Block* freeList[]) {
  toRemove->isFree = false;
  if (freeList[order] == toRemove) {
    freeList[order] = toRemove->next;
    return;
  }
  Block* currentBlock = freeList[order];
  while (currentBlock != NULL && currentBlock->next != toRemove) {
    currentBlock = currentBlock->next;
  }
  if (currentBlock != NULL) {
    currentBlock->next = toRemove->next;
  }
  toRemove->next = NULL;
}

static blockAlignment getAlignment(Block* block, void* heapStart) {
  if ((((uint32_t)((uint8_t*)block - (uint8_t*)heapStart) / block->size) % 2) == 0) return LEFT;
  return RIGHT;
}

static void mergeBlock(Block* block, uint32_t order, void* heapStart, uint64_t maxMem, Block* freeList[]) {
  if (block->size == maxMem) {
    freeList[order] = block;
    return;
  }

  blockAlignment blockAlignment = getAlignment(block, heapStart);

  Block* buddy = (blockAlignment == LEFT) ? (Block*)((char*)block + block->size) : (Block*)((char*)block - block->size);

  if (buddy == NULL || !buddy->isFree || buddy->size != block->size) {
    block->next = freeList[order];
    freeList[order] = block;
    return;
  }

  removeFromFreeList(block, order, freeList);
  removeFromFreeList(buddy, order, freeList);

  if (blockAlignment == RIGHT) block = buddy;

  block->size *= 2;
  block->isFree = true;
  mergeBlock(block, order + 1, heapStart, maxMem, freeList);
}

void globalFree(void* ptr) {
  if (ptr == NULL) return;
  Block* block = (Block*)ptr - 1;
  block->isFree = true;
  int32_t order = getOrder(block->size);
  mergeBlock(block, order, iniAddress, MAX_MEMORY_AVAILABLE, freeList);
}

void free(void* ptr) {
  if (ptr == NULL) return;
  PCB* pcb = fetchCurrentPCB();
  if (ptr < pcb->heap || ptr >= pcb->heap + PROCESS_HEAP_SIZE) return;
  Block* block = (Block*)ptr - 1;
  block->isFree = true;
  int32_t order = getOrder(block->size);
  mergeBlock(block, order, pcb->heap, PROCESS_HEAP_SIZE, pcb->freeList);
}

char* internalGetMemoryState(int32_t orderCount, int32_t heapSize, Block* freeList[]) {
  static char* unit = " B ";
  char* toReturn = malloc(MAX_STRING_SIZE);
  if (toReturn == NULL) return NULL;
  int32_t i = strcpy(toReturn, "Total: ");
  i += uintToBase(heapSize, toReturn + i, 10);
  i += strcpy(toReturn + i, unit);

  uint32_t totalFreeMemory = 0;
  uint32_t totalBlocks = 0;
  Block* currentBlock;
  for (int32_t j = 0; j < orderCount; j++) {
    currentBlock = freeList[j];
    while (currentBlock != NULL) {
      totalFreeMemory += currentBlock->size;
      currentBlock = currentBlock->next;
      ++totalBlocks;
    }
  }
  i += strcpy(toReturn + i, "| Used: ");
  i += uintToBase(heapSize - totalFreeMemory, toReturn + i, 10);
  i += strcpy(toReturn + i, unit);

  i += strcpy(toReturn + i, "| Unused: ");
  i += uintToBase(totalFreeMemory, toReturn + i, 10);
  i += strcpy(toReturn + i, unit);
  i += strcpy(toReturn + i, "in ");
  i += uintToBase(totalBlocks, toReturn + i, 10);
  i += strcpy(toReturn + i, " blocks ");
  toReturn[i] = 0;

  return toReturn;
}

char* getGlobalMemoryState() {
  return internalGetMemoryState(ORDER_COUNT, MAX_MEMORY_AVAILABLE, freeList);
}

char* getProcessMemoryState(uint32_t pid) {
  PCB* pcb = getPCB(pid);
  if (pcb == NULL) return NULL;
  return internalGetMemoryState(PROCESS_HEAP_ORDER_COUNT, PROCESS_HEAP_SIZE, pcb->freeList);
}

#endif
