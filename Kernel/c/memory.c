#include <memory.h>
#include <stdio.h>
#include <stdint.h>



// extern uint8_t endOfBinary;

// 64MB max heap size
const int heapSize = (1 << 20) * 64;
static void* heapStart;
static void* heapCurrent;

static const uint64_t addressByteSize = sizeof(void*);
/*
heapStart alignment example:

If endOfModules = 0x503e3 => heapStart = 0x503e8
0x00000000000503d8  00 00 00 00 00 00 00 00
0x00000000000503e0  00 00 00 xx 00 00 00 00
0x00000000000503e8  xx 00 00 00 00 00 00 00
0x00000000000503f0  00 00 00 00 00 00 00 00

endOfModules = 0x503e3        v                 
0x00000000000503e0  00 00 00 00 00 00 00 00

heapStart = 0x503e3 + 7
heapStart = 0x503ea        v                        
0x00000000000503e8  00 00 00 00 00 00 00 00

mask = ~7 = 0b1111...11111000 (with a total of 64 bits)
heapStart = heapStart & mask = 0b...1010 & 0b...1000 = 0b...1000
heapStart = 0x503e8  v                              
0x00000000000503e8  00 00 00 00 00 00 00 00
*/

#define MINIMUM_BLOCK_SIZE ((uint64_t) (alinedHeapStructSize << 1))

#define heapBITS_PER_BYTE ((uint64_t) 8)
#define heapBLOCK_ALLOCATED_BITMASK (((uint64_t)1) << ((sizeof(uint64_t) * heapBITS_PER_BYTE) - 1))
#define heapBLOCK_SIZE_IS_VALID(blockSize) (((blockSize) & heapBLOCK_ALLOCATED_BITMASK) == 0)
#define heapBLOCK_IS_ALLOCATED(block) (((block->blockSize) & heapBLOCK_ALLOCATED_BITMASK) != 0)
#define heapALLOCATE_BLOCK(block) ((block->blockSize) |= heapBLOCK_ALLOCATED_BITMASK)
#define heapFREE_BLOCK(block) ((block->blockSize) &= ~heapBLOCK_ALLOCATED_BITMASK)

typedef struct blockLink_t{
  struct blockLink_t * nextFreeBlock;
  uint64_t blockSize;
} blockLink_t;

// 64MB max heap size
static const int heapSize = (1 << 20) * 64;

static void* heapStart;
static void* heapCurrent;
static blockLink_t listStart;
static blockLink_t * listEnd = NULL;

static const uint64_t addressByteSize = sizeof(void*);
static const uint64_t alinedHeapStructSize = (sizeof(blockLink_t) + addressByteSize - 1) & ~(addressByteSize - 1);

static uint64_t freeBytesRemaining;
static uint64_t numberOfSuccessfulAllocations = 0;
static uint64_t numberOfSuccessfulFrees = 0;

void memoryInit(void * endOfModules) {
  void * alinedHeapStart = (void*)(((uint64_t)endOfModules + addressByteSize - 1) & ~(addressByteSize - 1));
  listStart.nextFreeBlock = alinedHeapStart;
  listStart.blockSize = 0;
  void * alinedHeapEnd = (void*)((uint64_t) (alinedHeapStart + heapSize - alinedHeapStructSize) & ~(addressByteSize - 1));
  listEnd = (blockLink_t *) alinedHeapEnd;
  listEnd->blockSize = 0;
  listEnd->nextFreeBlock = NULL;

  blockLink_t * firstFreeBlock = (blockLink_t *) alinedHeapStart;
  firstFreeBlock->blockSize = (uint64_t) (alinedHeapEnd - alinedHeapStart);
  firstFreeBlock->nextFreeBlock = listEnd;

  freeBytesRemaining = firstFreeBlock->blockSize;
}

static void insertBlockIntoFreeList(blockLink_t * blockToInsert){
  blockLink_t * blockIterator;
    uint8_t *puc;

    /* Iterate through the list until a block is found that has a higher address
     * than the block being inserted. */
    for (blockIterator = &listStart; blockIterator->nextFreeBlock < blockToInsert; blockIterator = blockIterator->nextFreeBlock)
    {
        /* Nothing to do here, just iterate to the right position. */
    }

    /* Do the block being inserted, and the block it is being inserted after
     * make a contiguous block of memory? */
    puc = (uint8_t *) blockIterator;

    if ((puc + blockIterator->blockSize) == (uint8_t *)blockToInsert)
    {
        blockIterator->blockSize += blockToInsert->blockSize;
        blockToInsert = blockIterator;
    }
    else
    {
        mtCOVERAGE_TEST_MARKER();
    }

    /* Do the block being inserted, and the block it is being inserted before
     * make a contiguous block of memory? */
    puc = (uint8_t *)blockToInsert;

    if ((puc + blockToInsert->blockSize) == (uint8_t *) blockIterator->nextFreeBlock)
    {
        if (blockIterator->nextFreeBlock != listEnd)
        {
            /* Form one big block from the two blocks. */
            blockToInsert->blockSize += blockIterator->nextFreeBlock->blockSize;
            blockToInsert->nextFreeBlock = blockIterator->nextFreeBlock->nextFreeBlock;
        }
        else
        {
            blockToInsert->nextFreeBlock = listEnd;
        }
    }
    else
    {
        blockToInsert->nextFreeBlock = blockIterator->nextFreeBlock;
    }

    /* If the block being inserted plugged a gab, so was merged with the block
     * before and the block after, then it's pxNextFreeBlock pointer will have
     * already been set, and should not be set here as that would make it point
     * to itself. */
    if (blockIterator != blockToInsert)
    {
        blockIterator->nextFreeBlock = blockToInsert;
    }
    else
    {
        mtCOVERAGE_TEST_MARKER();
    }
}



void* malloc(uint64_t size) {
  
  blockLink_t * block;
  blockLink_t * previousBlock;
  blockLink_t * newBlockLink;
  void * toReturn;
  uint64_t alinedRequiredSize = 0;

  if(size > 0){
    alinedRequiredSize = (size + alinedHeapStructSize + addressByteSize - 1) & ~(addressByteSize - 1);
  }

  if(heapBLOCK_SIZE_IS_VALID(alinedRequiredSize) && alinedRequiredSize > 0 && alinedRequiredSize <= freeBytesRemaining){
    previousBlock = &listStart;
    block = listStart.nextFreeBlock;

    while ((block->blockSize < alinedRequiredSize) && (block->nextFreeBlock != NULL)){
      previousBlock = block;
      block = block->nextFreeBlock;
    }

    if(block != listEnd){
      toReturn = (void *)(previousBlock->nextFreeBlock + alinedHeapStructSize);

      previousBlock->nextFreeBlock = block->nextFreeBlock;

      if((block->blockSize - alinedRequiredSize) > MINIMUM_BLOCK_SIZE){
        newBlockLink = (void *) (block + alinedRequiredSize);
        newBlockLink->blockSize = block->blockSize - alinedRequiredSize;
        block->blockSize = alinedRequiredSize;

        insertBlockIntoFreeList(newBlockLink);
      }

      freeBytesRemaining -= block->blockSize;

      heapALLOCATE_BLOCK(block);
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

void free(void * ptr){
  uint8_t *puc = (uint8_t *) ptr;
    blockLink_t *pxLink;

    if (ptr != NULL)
    {
        /* The memory being freed will have an BlockLink_t structure immediately
         * before it. */
        puc -= alinedHeapStructSize;

        /* This casting is to keep the compiler from issuing warnings. */
        pxLink = (void *)puc;

        if (heapBLOCK_IS_ALLOCATED(pxLink) != 0)
        {
            if (pxLink->nextFreeBlock == NULL)
            {
                /* The block is being returned to the heap - it is no longer
                 * allocated. */
                heapFREE_BLOCK(pxLink);

                
                    /* Add this block to the list of free blocks. */
                freeBytesRemaining += pxLink->blockSize;
                insertBlockIntoFreeList(((blockLink_t *)pxLink));
                numberOfSuccessfulFrees++;
            
            }
            else
            {
                mtCOVERAGE_TEST_MARKER();
            }
        }
        else
        {
            mtCOVERAGE_TEST_MARKER();
        }
    }
}