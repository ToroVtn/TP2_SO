#include <memory.h>
#include <lib.h>

// 4KB stack size
static const int stackSize = (1 << 10) * 4;

static const uint64_t addressByteSize = sizeof(void*);

/*
rsp = e                e                   
0x00000000000503d8  00 00 00 00 00 00 00 00
0x00000000000503e0  00 00 00 00 00 00 00 00
0x00000000000503e8  00 00 00 00 00 00 00 00
                                   s  >>   
0x00000000000503f0  00 00 00 00 00 00 00 00

rsp = e - 8            e                   
0x00000000000503d8  00 00 00 00 00 00 00 00
0x00000000000503e0  00 00 00 00 00 00 00 00
                                   v       
0x00000000000503e8  00 00 00 00 00 00 00 00
                                   s  >>   
0x00000000000503f0  00 00 00 00 00 00 00 00

rsp = (e - 8) & ~7
                       e                 
0x00000000000503d8  00 00 00 00 00 00 00 00
0x00000000000503e0  00 00 00 00 00 00 00 00
                    v                      
0x00000000000503e8  00 00 00 00 00 00 00 00
                                    s >>   
0x00000000000503f0  00 00 00 00 00 00 00 00
 */

 void* realloc(void* ptr, uint64_t oldSize, uint64_t newSize) {
  void* mem = globalMalloc(newSize);
  if (mem == NULL) return NULL;
  memcpy(mem, ptr, oldSize);
  globalFree(ptr);
  return mem;
}

void allocateStack(void** stackBase, void** stackTop) {
  *stackTop = globalMalloc(stackSize);
  *stackBase = *stackTop + stackSize - 1;
  *stackBase = (void*)(((uint64_t)*stackBase - addressByteSize) & ~(addressByteSize - 1));
}