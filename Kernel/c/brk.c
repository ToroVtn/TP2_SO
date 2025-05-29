#include <brk.h>
#include <stdio.h>
#include <stdint.h>

#define HEAP_SIZE (1 << 20) * 64 //64 * 1MB heap

static void *heap_start;
static void *heap_end;          // Current program break
static void *heap_limit;

static const __uint64_t addressByteSize = sizeof(void*);

void buddy_init(void * endOfModules){  
    heap_start = (void*)(( (__uint64_t) endOfModules + addressByteSize - 1) & ~(addressByteSize - 1));
    heap_end = heap_start;          // Current program break
    heap_limit = heap_start + HEAP_SIZE;  // Max limit  
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