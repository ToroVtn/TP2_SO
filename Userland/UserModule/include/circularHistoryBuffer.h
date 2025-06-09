#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <arrayUtils.h>
#include <stdint.h>

typedef struct CircularHistoryBufferADT* CircularHistoryBuffer;

CircularHistoryBuffer initCHB(uint64_t elementSize, uint64_t size, ElementDestructor ElementDestructor, CompareEleFn cmpEleFn);
void pushToCHB(CircularHistoryBuffer cb, void* ele);
void* readNextFromCHB(CircularHistoryBuffer cb);
void* readPrevFromCHB(CircularHistoryBuffer cb);
void readRestFromCHB(CircularHistoryBuffer cb);
void moveTofrontOrPushCHB(CircularHistoryBuffer cb, void* ele);
uint64_t getCHBLen(CircularHistoryBuffer cb);
uint64_t getCHBSize(CircularHistoryBuffer cb);
void freeCHB(CircularHistoryBuffer cb);
void CHB_printState(CircularHistoryBuffer cb);
bool containsCHB(CircularHistoryBuffer cb, void* ele);

#endif