#ifndef ARRAY_ADT_H
#define ARRAY_ADT_H

typedef struct ArrayCDT* Array;

#include <stdbool.h>
#include <stdint.h>

typedef void (*ElementDestructor)(void* elem);
typedef void (*MapFn)(void* mappedElem, void* ele, uint64_t idx);

void* initArray(uint64_t elemSize, uint64_t initialCapacity, ElementDestructor ElementDestructor);
bool freeArray(Array arr);
int64_t pushToArray(Array arr, const void* elem);
bool popAndGetFromArray(Array arr, void* elem);
bool popArray(Array arr);
bool emptyArray(Array arr);
uint64_t arrayLen(Array arr);
void* getAtArrayIdx(Array arr, int64_t idx);
bool arrayCopyInto(Array arr, int64_t idx, const void* elemArray, uint64_t length);
bool setAtArrayIdx(Array arr, int64_t idx, void* elem);
Array CArrayToMyAarray(const void* array, uint64_t length, uint64_t elemSize, ElementDestructor freeFn);
bool concatArray(Array destiny, Array source);
const void* arrayData(Array arr);
void* arrayCopyTo(Array arr, void* array);
void* arrayCloneAsCArray(Array arr);

#endif