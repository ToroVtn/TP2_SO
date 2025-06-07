//based on freeRTOS heap_4.c

#include <memory.h>
#include <scheduler.h>
#include <stdint.h>


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

void insertBlockIntoFreeList(Block * block);

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

        insertBlockIntoFreeList(newBlockLink);
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
    return internalMalloc(size, pcb->listStart, pcb->listEnd, &(pcb->freeBytes));
}

void internalFree( void* ptr, Block* listStart, Block* listEnd, size_t* freeBytes ){
    if( ptr == NULL ) return;

    // Block info is before the ptr like a header
    // uint8_t * puc = ( uint8_t * ) ptr - block_size;
    Block * toFree = (Block *) ptr - 1;

    if( BLOCK_IS_ALLOCATED( toFree ) && toFree->nextFreeBlock == NULL ){
        FREE_BLOCK( toFree );
        *freeBytes += toFree->blockSize;
        insertBlockIntoFreeList( ( ( Block * ) toFree ) );
    }
}

void globalFree(void* ptr) {
  internalFree(ptr, &listStart, listEnd, &freeBytes);
}

void free(void* ptr) {
  if (ptr == NULL) return;
  PCB* pcb = fetchCurrentPCB();
  if (ptr < pcb->heap || ptr >= pcb->heap + PROCESS_HEAP_SIZE) return;
  internalFree(ptr, pcb->listStart, pcb->listEnd, &(pcb->freeBytes));
}

//FUNCION PROVISORIA
void * realloc( void * ptr, uint64_t oldSize, uint64_t newSize ){
    if( ptr == NULL ){
        return NULL;
    }
    
    if( newSize == 0 ){
        free( ptr );
        return NULL;
    }
    
    if( oldSize == newSize ){
        return ptr;
    }
    
    void * newPtr = malloc( newSize );

    if(newPtr == NULL){
        return NULL;
    }
    
    uint64_t copySize = (oldSize < newSize) ? oldSize : newSize;        
    newPtr = memcpy(newPtr, ptr, copySize);
    free( ptr );
    
    return newPtr;
}

void insertBlockIntoFreeList(Block * block){
    Block * blockIt;
    uint8_t * puc; // aux for adress pointer adition

    // find a blockIt that has a higher adress than block 
    for( blockIt = &listStart; blockIt->nextFreeBlock < block; blockIt = blockIt->nextFreeBlock ){}

    // check if blockIt and block are contiguous
    puc = ( uint8_t * ) blockIt;

    if( ( puc + blockIt->blockSize ) == ( uint8_t * ) block ){
        blockIt->blockSize += block->blockSize;
        block = blockIt;
    }

    // check if block is contiguous with blockIt->nextFreeBlock
    puc = ( uint8_t * ) block;

    if( ( puc + block->blockSize ) == ( uint8_t * ) blockIt->nextFreeBlock ){
        if( blockIt->nextFreeBlock != listEnd ){
            // merge them
            block->blockSize += blockIt->nextFreeBlock->blockSize;
            block->nextFreeBlock = blockIt->nextFreeBlock->nextFreeBlock;
        } else {
            block->nextFreeBlock = listEnd;
        }
    } else {
        block->nextFreeBlock = blockIt->nextFreeBlock;
    }

    // if there was no contiguity, simply add
    if( blockIt != block ){
        blockIt->nextFreeBlock = block;
    }
}

#endif