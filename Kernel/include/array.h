#ifndef ARRAY_ADT_H
#define ARRAY_ADT_H

typedef struct ArrayCDT* Array;

#include <stdbool.h>
#include <stdint.h>

typedef void (*FreeEleFn)(void* ele);
typedef void (*MapFn)(void* mappedEle, void* ele, uint64_t idx);

void* Array_initialize(uint64_t elementSize, uint64_t initialCapacity, FreeEleFn freeEleFn);
bool Array_free(Array a);
int64_t Array_push(Array a, const void* ele);
bool Array_popGetEle(Array a, void* ele);
bool Array_pop(Array a);
bool Array_clear(Array a);
uint64_t Array_getLen(Array a);
void* Array_get(Array a, int64_t idx);
bool Array_setn(Array a, int64_t idx, const void* eleArray, uint64_t length);
bool Array_set(Array a, int64_t idx, void* ele);
Array Array_fromVanillaArray(const void* array, uint64_t length, uint64_t elementSize, FreeEleFn freeFn);
bool Array_concat(Array dst, Array src);
const void* Array_getVanillaArray(Array a);
void* Array_copyVanillaArrayInto(Array a, void* array);
void* Array_getVanillaArrayCopy(Array a);

#endif