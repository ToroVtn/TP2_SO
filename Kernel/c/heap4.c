#include <memory.h>
#include <scheduler.h>
#include <memory.h>
#include <scheduler.h>

#ifndef BUDDY

#define HEAP_SIZE (1<<20) * 64

#define BITS_PER_BYTE ( ( size_t ) 8 )

static const size_t block_size = sizeof(Block);
static const size_t addressByteSize = sizeof(void*);

#define MINIMUM_BLOCK_SIZE    ( ( size_t ) ( block_size << 1 ) )

#define BLOCK_ALLOCATED_BITMASK    ( ( ( size_t ) 1 ) << ( ( sizeof( size_t ) * BITS_PER_BYTE ) - 1 ) )
#define BLOCK_SIZE_IS_VALID( blockSize )    ( ( ( blockSize ) & BLOCK_ALLOCATED_BITMASK ) == 0 )
#define BLOCK_IS_ALLOCATED( block )        ( ( ( block->blockSize ) & BLOCK_ALLOCATED_BITMASK ) != 0 )
#define ALLOCATE_BLOCK( block )            ( ( block->blockSize ) |= BLOCK_ALLOCATED_BITMASK )
#define FREE_BLOCK( block )                ( ( block->blockSize ) &= ~BLOCK_ALLOCATED_BITMASK )

#define ADD_WILL_OVERFLOW( a, b )          ( ( a ) > ( HEAP_SIZE - ( b ) ) )

#define SUBTRACT_WILL_UNDERFLOW( a, b )    ( ( a ) < ( b ) )

/*Block * listStart and using it as listStart->whatever crashes when calling 
to malloc ("cannot access memory adress" error when entering while; checked with gdb) */
static Block listStart;
static Block * listEnd = NULL;

static size_t freeBytes;

static void insertBlockIntoFreeList(Block * blockToInsert, Block* listStart, Block* listEnd){
  Block * blockIterator;
  uint8_t *aux;

  // Iterate through the list until a block is found that has a higher address than the block being inserted. 
  for (blockIterator = listStart; blockIterator->nextFreeBlock < blockToInsert; blockIterator = blockIterator->nextFreeBlock) {}

  // Check if block inserted is after blockIterator
  aux = (uint8_t *) blockIterator;

  if ((aux + blockIterator->blockSize) == (uint8_t *)blockToInsert) {
      blockIterator->blockSize += blockToInsert->blockSize;
      blockToInsert = blockIterator;
  }
  // Check if block inserted is right before blockIterator->next
  aux = (uint8_t *)blockToInsert;

  if ((aux + blockToInsert->blockSize) == (uint8_t *) blockIterator->nextFreeBlock) {
      if (blockIterator->nextFreeBlock != listEnd) {
          /* Form one big block from the two blocks. */
          blockToInsert->blockSize += blockIterator->nextFreeBlock->blockSize;
          blockToInsert->nextFreeBlock = blockIterator->nextFreeBlock->nextFreeBlock;
      }
      else {
          blockToInsert->nextFreeBlock = listEnd;
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

void internalListInit(void* heapStart, uint32_t heapSize, Block* listStart, Block** listEnd, size_t* freeBytes){
    void * alignedHeapStart = (void*) ( ( (size_t) heapStart + addressByteSize - 1) & ~(addressByteSize - 1));
    listStart->nextFreeBlock = (Block *) alignedHeapStart;
    listStart->blockSize = 0;

    void * alignedHeapEnd = (void*)((size_t) (alignedHeapStart + HEAP_SIZE - block_size));
    *listEnd = (Block *) alignedHeapEnd;
    (*listEnd)->blockSize = 0;
    (*listEnd)->nextFreeBlock = NULL;

    Block * firstFreeBlock = (Block *) alignedHeapStart;
    firstFreeBlock->blockSize = (size_t) (alignedHeapEnd - alignedHeapStart);
    firstFreeBlock->nextFreeBlock = *listEnd;

    *freeBytes = firstFreeBlock->blockSize;
}

void listInit(void* heapStart, Block* listStart, Block** listEnd, size_t* freeBytes) {
    internalListInit(heapStart, PROCESS_HEAP_SIZE, listStart, listEnd, freeBytes);
}
  
void memoryInit(void* endOfModules){
    internalListInit(endOfModules, HEAP_SIZE, &listStart, &listEnd, &freeBytes);
}

void * internalMalloc( size_t request, Block * listStart, Block * listEnd, size_t * freeBytes ){
    if (request <= 0) return NULL;

    Block * block;
    Block * previousBlock;
    Block * newBlockLink;
    void * toReturn = NULL;
    size_t alignedRequiredSize = (request + block_size + addressByteSize - 1) & ~(addressByteSize - 1);

    if( alignedRequiredSize > *freeBytes || alignedRequiredSize <= 0 || !BLOCK_SIZE_IS_VALID(alignedRequiredSize)) return NULL;

    previousBlock = listStart;
    block = listStart->nextFreeBlock;
    while( block->blockSize < alignedRequiredSize && block->nextFreeBlock != NULL ){
        previousBlock = block;
        block = block->nextFreeBlock;
    }

    if(block == listEnd) return NULL;
    
    // useful memory after Block data
    // ( ( ( uint8_t * ) previousBlock->nextFreeBlock ) + block_size )
    toReturn = ( void * ) (previousBlock->nextFreeBlock + 1);

    // take off freeList
    previousBlock->nextFreeBlock = block->nextFreeBlock;

    if( ( block->blockSize - alignedRequiredSize ) > MINIMUM_BLOCK_SIZE ){
        // split block in two for adequate size
        newBlockLink = ( Block * ) ( ( uint8_t * ) block + alignedRequiredSize );
        newBlockLink->blockSize = block->blockSize - alignedRequiredSize;
        block->blockSize = alignedRequiredSize;

        insertBlockIntoFreeList(newBlockLink, listStart, listEnd);
    }

    *freeBytes -= block->blockSize;

    block->nextFreeBlock = NULL;

    return toReturn;
}

void* globalMalloc(size_t size) {
    return internalMalloc(size, &listStart, listEnd, &freeBytes);
}
  
void* malloc(size_t size) {
    PCB* pcb = fetchCurrentPCB();
    return internalMalloc(size, pcb->listStart, pcb->listEnd, &(pcb->bytesAvailable));
}

void internalFree(void* ptr, Block* listStart, Block* listEnd, uint64_t* freeBytes) {
  if( ptr == NULL) return;
  // The block structure is before the useful memory
  Block *freeBlock = (Block *) ptr - 1;

  if (BLOCK_IS_ALLOCATED(freeBlock) && freeBlock->nextFreeBlock == NULL) {
      FREE_BLOCK(freeBlock);
      *freeBytes += freeBlock->blockSize;
      insertBlockIntoFreeList(((Block *)freeBlock), listStart, listEnd);
  }
}

void globalFree(void* ptr) {
  internalFree(ptr, &listStart, listEnd, &freeBytes);
}

void free(void* ptr) {
  if (ptr == NULL) return;
  PCB* pcb = fetchCurrentPCB();
  if (ptr < pcb->heap || ptr >= pcb->heap + PROCESS_HEAP_SIZE) return;
  internalFree(ptr, pcb->listStart, pcb->listEnd, &(pcb->bytesAvailable));
}


char* internalGetMemoryState(int32_t heapSize, uint64_t* bytesAvailable) {
  heapSize -= block_size;
  static char* unit = " B ";
  char* toReturn = malloc(MAX_STRING_SIZE);
  if (toReturn == NULL) return NULL;
  int32_t i = strcpy(toReturn, "Total: ");
  i += uintToBase(heapSize, toReturn + i, 10);
  i += strcpy(toReturn + i, unit);
  i += strcpy(toReturn + i, "| Allocated: ");
  i += uintToBase(heapSize - *bytesAvailable, toReturn + i, 10);
  i += strcpy(toReturn + i, unit);
  i += strcpy(toReturn + i, "| Free: ");
  i += uintToBase(*bytesAvailable, toReturn + i, 10);
  i += strcpy(toReturn + i, unit);
  toReturn[i] = 0;

  return toReturn;
}

char* getGlobalMemoryState() {
  return internalGetMemoryState(HEAP_SIZE, &freeBytes);
}

char* getProcessMemoryState(uint32_t pid) {
  PCB* pcb = getPCB(pid);
  if (pcb == NULL) return NULL;
  return internalGetMemoryState(PROCESS_HEAP_SIZE, &(pcb->bytesAvailable));
}

#endif
