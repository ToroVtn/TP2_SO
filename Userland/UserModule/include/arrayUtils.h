#ifndef ARRAY_ADT_H
#define ARRAY_ADT_H

typedef struct ArrayCDT* Array;

#include <stdbool.h>
#include <stdint.h>

typedef void (*ElementDestructor)(void* ele);
typedef int32_t (*CompareEleFn)(void* ele1, void* ele2);
typedef void (*MapFn)(void* mappedEle, void* ele, uint64_t idx);

void* initArray(uint64_t elementSize, uint64_t initialCapacity, ElementDestructor elementDestructor, CompareEleFn cmpEleFn);
void freeArray(Array a);
void pushToArray(Array a, const void* ele);
bool popAndGetfromArray(Array a, void* ele);
void popArray(Array a);
void emptyArray(Array a);
uint64_t arrayLen(Array a);
void* getAtArrayIdx(Array a, int64_t idx);
void copyElemAt(Array a, int64_t idx, uint64_t n, void* eleArr);
void arrayCopyInto(Array a, int64_t idx, const void* eleArray, uint64_t length, bool free);
void setAtArrayIdx(Array a, int64_t idx, void* ele);
void arrayInfo(Array a);
void concatArray(Array dst, Array src);
const void* arrayData(Array a);
void arrayCopyTo(Array a, void* eleArr);
void* arrayCloneAsCArray(Array a);
bool equalsArray(Array a1, Array a2);
int32_t findArray(Array a, void* ele);
bool hasArray(Array a, void* ele);
void removeFromArray(Array a, int64_t idx);
uint64_t getElemSize(Array a);


#endif