#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <array.h>
#include <stdint.h>

typedef struct CircularHistoryBufferADT* CircularHistoryBuffer;

CircularHistoryBuffer CHB_initialize(uint64_t elementSize, uint64_t size, FreeEleFn freeEleFn, CompareEleFn cmpEleFn);
void CHB_push(CircularHistoryBuffer cb, void* ele);
void* CHB_readNext(CircularHistoryBuffer cb);
void* CHB_readPrev(CircularHistoryBuffer cb);
void CHB_readRest(CircularHistoryBuffer cb);
void CHB_moveToFrontOrPush(CircularHistoryBuffer cb, void* ele);
uint64_t CHB_getLen(CircularHistoryBuffer cb);
uint64_t CHB_getSize(CircularHistoryBuffer cb);
void CHB_free(CircularHistoryBuffer cb);
void CHB_printState(CircularHistoryBuffer cb);
bool CHB_has(CircularHistoryBuffer cb, void* ele);

#endif