#include <brk.h>

#define HEAP_SIZE (1 << 20) // 1MB heap

static uint8_t * simulated_heap;
static uint8_t *heap_start;
static uint8_t *heap_end;          // Current program break
static uint8_t *heap_limit;

static const uint64_t addressByteSize = sizeof(void*);

void buddy_init(void * endOfModules){  
    simulated_heap = (void*)(( (uint64_t) endOfModules + addressByteSize - 1) & ~(addressByteSize - 1));
    heap_start = simulated_heap;
    heap_end = simulated_heap;          // Current program break
    heap_limit = simulated_heap + HEAP_SIZE;  // Max limit  
}

void *sbrk(intptr_t increment) {
  uint8_t *old_break = heap_end;

  if (increment < 0 || heap_end + increment > heap_limit) {
    return (void *)-1;  // Simulate failure
  }

  heap_end += increment;
  return old_break;
}

int brk(void *new_break) {
  if ((uint8_t *)new_break < heap_start || (uint8_t *)new_break > heap_limit) {
    return -1;
  }

  heap_end = (uint8_t *)new_break;
  return 0;
}