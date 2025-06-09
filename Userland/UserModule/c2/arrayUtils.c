#include "../include/arrayUtils.h"
#include <stdint.h>
#include <stdlib.h>
#include <syscalls.h>
#include <utils.h>

typedef struct ArrayCDT {
  // Use uint8_t* instead of void* because void* can't be used in arithmetic operations.
  uint8_t* array;
  uint64_t elementSize;
  uint64_t capacity;
  uint64_t length;
  ElementDestructor freeElemFn;
  CompareEleFn cmpEleFn;
} ArrayCDT;

void copynEleAt(Array a, uint64_t idx, const void* eleArr, uint64_t n);
void copyEleAt(Array a, uint64_t idx, const void* ele);
void growTo(Array a, uint64_t newCapacity);
void growBy(Array a, uint64_t extraCapacity);
int64_t toRealIdx(Array a, int64_t idx);

void* initArray(uint64_t elementSize, uint64_t initialCapacity, ElementDestructor freeElemFn, CompareEleFn cmpEleFn) {
  if (elementSize == 0) exitWithError("@initArray elementSize can't be 0");
  ArrayCDT* a = sysMalloc(sizeof(ArrayCDT));
  if (a == NULL) exitWithError("@initArray malloc error");
  a->capacity = initialCapacity ? initialCapacity : 1;
  a->array = sysMalloc(a->capacity * elementSize);
  if (a->array == NULL) {
    sysFree(a);
    exitWithError("@initArray malloc error");
  }
  a->length = 0;
  a->elementSize = elementSize;
  a->freeElemFn = freeElemFn;
  a->cmpEleFn = cmpEleFn;
  return a;
}

void freeArray(Array a) {
  if (a == NULL) exitWithError("@freeArray Array instance can't be NULL");

  if (a->freeElemFn != NULL) {
    for (int32_t i = 0; i < a->length; ++i) a->freeElemFn(getAtArrayIdx(a, i));
  }
  sysFree(a->array);
  sysFree(a);
}

void pushToArray(Array a, const void* ele) {
  if (a == NULL) {
    exitWithError("@pushToArray Array instance can't be NULL");
  } else if (ele == NULL) exitWithError("@pushToArray element to push can't be NULL");

  if (a->length >= a->capacity) growBy(a, a->capacity);

  copyEleAt(a, a->length, ele);
  ++a->length;
}

bool popAndGetFromArray(Array a, void* ele) {
  if (a == NULL) exitWithError("@popArray Array instance can't be NULL");
  if (a->length == 0) return false;

  void* eleToPop = getAtArrayIdx(a, -1);
  sysMemcpy(ele, eleToPop, a->elementSize);
  --a->length;
  return true;
}

void popArray(Array a) {
  if (a == NULL) exitWithError("@popArray Array instance can't be NULL");
  if (a->length == 0) return;
  if (a->freeElemFn != NULL) a->freeElemFn(getAtArrayIdx(a, -1));
  --a->length;
}

void arrayCopyInto(Array a, int64_t idx, const void* eleArray, uint64_t length, bool free) {
  if (a == NULL) exitWithError("@arrayCopyInto Array instance can't be NULL");
  idx = toRealIdx(a, idx);
  if (idx < 0) exitWithError("@arrayCopyInto idx outside of bounds");
  if (length <= 0) return;

  if (free && a->freeElemFn != NULL) {
    for (int32_t i = idx; i < idx + length; ++i) {
      a->freeElemFn(getAtArrayIdx(a, i));
    }
  }

  if (length > a->length - idx) growBy(a, length - (a->length - idx));
  copynEleAt(a, idx, eleArray, length);
}

void setAtArrayIdx(Array a, int64_t idx, void* ele) {
  arrayCopyInto(a, idx, ele, 1, true);
}

void* getAtArrayIdx(Array a, int64_t idx) {
  if (a == NULL) return NULL;
  idx = toRealIdx(a, idx);
  if (idx < 0) return NULL;
  return a->array + idx * a->elementSize;
}

void copyElemAt(Array a, int64_t idx, uint64_t n, void* eleArr) {
  if (a == NULL) exitWithError("@copyElemAt Array instance can't be NULL");
  idx = toRealIdx(a, idx);
  if (idx < 0) exitWithError("@copyElemAt index out of bounds");
  if (idx + n > a->length) exitWithError("@copyElemAt number of elements out of bounds");
  sysMemcpy(eleArr, a->array + idx * a->elementSize, n * a->elementSize);
}

void emptyArray(Array a) {
  if (a == NULL) exitWithError("@emptyArray Array instance can't be NULL");
  if (a->freeElemFn != NULL) {
    for (int32_t i = 0; i < a->length; ++i) {
      a->freeElemFn(getAtArrayIdx(a, i));
    }
  }
  a->length = 0;
}

uint64_t arrayLen(Array a) {
  if (a == NULL) exitWithError("@arrayLen Array instance can't be NULL");
  return a->length;
}

void arrayInfo(Array a) {
  if (a == NULL) exitWithError("@arrayInfo Array instance can't be NULL");
  printf("{ \n");
  printf(
      "  length: %lu\n"
      "  elementSize: %lu\n"
      "  capacity: %lu\n"
      "  array: %p\n",
      a->length, a->elementSize, a->capacity, a->array
  );
  printf("}\n");
}

void concatArray(Array dst, Array src) {
  if (dst == NULL || src == NULL) exitWithError("@concatArray Array instances can't be NULL");
  if (dst->elementSize != src->elementSize) {
    exitWithError("@concatArray both Array instances should be of same element type");
  }

  uint64_t neededCapacity = dst->length + src->length;
  if (dst->capacity < neededCapacity) growTo(dst, neededCapacity);
  copynEleAt(dst, dst->length, src->array, src->length);
  dst->length += src->length;
}

const void* arrayData(Array a) {
  if (a == NULL) exitWithError("@arrayData Array instance can't be NULL");
  return a->array;
}

void arrayCopyTo(Array a, void* eleArr) {
  copyElemAt(a, 0, a->length, eleArr);
}

void* arrayCloneAsCArray(Array a) {
  if (a == NULL) exitWithError("@arrayCloneAsCArray Array instance can't be NULL");
  void* array = sysMalloc(a->length * a->elementSize);
  arrayCopyTo(a, array);
  return array;
}

bool equalsArray(Array a1, Array a2) {
  if (a1 == NULL || a2 == NULL) exitWithError("@equalsArray Array instances can't be NULL");
  if (a1->elementSize != a2->elementSize) exitWithError("@equalsArray Arrays must be of same element type");
  if (a1->cmpEleFn == NULL && a2->cmpEleFn == NULL) {
    exitWithError("@equalsArray One of the arrays must've been initalized with compare function");
  }

  CompareEleFn cmp = (a1->cmpEleFn != NULL) ? a1->cmpEleFn : a2->cmpEleFn;
  bool equal = true;
  for (int32_t i = 0; i < a1->length && equal; ++i) {
    void* ele1 = getAtArrayIdx(a1, i);
    void* ele2 = getAtArrayIdx(a2, i);
    if (cmp(ele1, ele2) != 0) equal = false;
  }

  return equal;
}

int32_t findArray(Array a, void* ele) {
  if (a == NULL) exitWithError("@findArray Array instances can't be NULL");
  if (a->cmpEleFn == NULL) exitWithError("@findArray Array must've been initalized with compare function");

  for (int32_t i = 0; i < a->length; ++i) {
    void* ele1 = getAtArrayIdx(a, i);
    if (a->cmpEleFn(ele, ele1) == 0) return i;
  }

  return -1;
}

bool hasArray(Array a, void* ele) {
  return findArray(a, ele) >= 0;
}

void removeFromArray(Array a, int64_t idx) {
  if (a == NULL) exitWithError("@removeFromArray Array instance can't be NULL");
  idx = toRealIdx(a, idx);
  if (idx < 0) exitWithError("@removeFromArray index out of bounds");
  if (idx == a->length - 1) {
    popArray(a);
    return;
  }
  if (a->freeElemFn != NULL) a->freeElemFn(getAtArrayIdx(a, idx));
  int32_t lenToCopy = a->length - (idx + 1);
  // Don't want to free the elements that will remain.
  arrayCopyInto(a, idx, a->array + (idx + 1) * a->elementSize, lenToCopy, false);
  --a->length;
}

uint64_t getElemSize(Array a) {
  if (a == NULL) exitWithError("@getElemSize Array instance can't be NULL");
  return a->elementSize;
}

//////////////////////////// Internal Functions ////////////////////////////

void copynEleAt(Array a, uint64_t idx, const void* eleArr, uint64_t n) {
  sysMemcpy(a->array + a->elementSize * idx, eleArr, a->elementSize * n);
}

void copyEleAt(Array a, uint64_t idx, const void* ele) {
  copynEleAt(a, idx, ele, 1);
}

void growTo(Array a, uint64_t newCapacity) {
  void* aux = realloc(a->array, a->capacity * a->elementSize, newCapacity * a->elementSize);
  if (aux == NULL) exitWithError("@Array's internal func growTo realloc error");
  a->capacity = newCapacity;
  a->array = aux;
}

void growBy(Array a, uint64_t extraCapacity) {
  growTo(a, a->capacity + extraCapacity);
}

int64_t toRealIdx(Array a, int64_t idx) {
  
  if (idx < 0) {
    
    if (-idx > a->length) return -1;
    idx += a->length;
  } else if (idx >= a->length) return -1;
  return idx;
}