#include <array.h>
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
  FreeEleFn freeEleFn;
  CompareEleFn cmpEleFn;
} ArrayCDT;

void copynEleAt(Array a, uint64_t idx, const void* eleArr, uint64_t n);
void copyEleAt(Array a, uint64_t idx, const void* ele);
void growTo(Array a, uint64_t newCapacity);
void growBy(Array a, uint64_t extraCapacity);
int64_t toRealIdx(Array a, int64_t idx);

void* Array_initialize(
    uint64_t elementSize, uint64_t initialCapacity, FreeEleFn freeEleFn, CompareEleFn cmpEleFn
) {
  if (elementSize == 0) exitWithError("@Array_initialize elementSize can't be 0");
  ArrayCDT* a = sysMalloc(sizeof(ArrayCDT));
  if (a == NULL) exitWithError("@Array_initialize malloc error");
  a->capacity = initialCapacity ? initialCapacity : 1;
  a->array = sysMalloc(a->capacity * elementSize);
  if (a->array == NULL) {
    sysFree(a);
    exitWithError("@Array_initialize malloc error");
  }
  a->length = 0;
  a->elementSize = elementSize;
  a->freeEleFn = freeEleFn;
  a->cmpEleFn = cmpEleFn;
  return a;
}

void Array_free(Array a) {
  if (a == NULL) exitWithError("@Array_free Array instance can't be NULL");

  if (a->freeEleFn != NULL) {
    for (int32_t i = 0; i < a->length; ++i) a->freeEleFn(Array_get(a, i));
  }
  sysFree(a->array);
  sysFree(a);
}

void Array_push(Array a, const void* ele) {
  if (a == NULL) {
    exitWithError("@Array_push Array instance can't be NULL");
  } else if (ele == NULL) exitWithError("@Array_push element to push can't be NULL");

  if (a->length >= a->capacity) growBy(a, a->capacity);

  copyEleAt(a, a->length, ele);
  ++a->length;
}

bool Array_popGetEle(Array a, void* ele) {
  if (a == NULL) exitWithError("@Array_pop Array instance can't be NULL");
  if (a->length == 0) return false;

  void* eleToPop = Array_get(a, -1);
  sysMemcpy(ele, eleToPop, a->elementSize);
  --a->length;
  return true;
}

void Array_pop(Array a) {
  if (a == NULL) exitWithError("@Array_pop Array instance can't be NULL");
  if (a->length == 0) return;
  if (a->freeEleFn != NULL) a->freeEleFn(Array_get(a, -1));
  --a->length;
}

void Array_setn(Array a, int64_t idx, const void* eleArray, uint64_t length, bool free) {
  if (a == NULL) exitWithError("@Array_setn Array instance can't be NULL");
  idx = toRealIdx(a, idx);
  if (idx < 0) exitWithError("@Array_setn idx outside of bounds");
  if (length <= 0) return;

  if (free && a->freeEleFn != NULL) {
    for (int32_t i = idx; i < idx + length; ++i) {
      a->freeEleFn(Array_get(a, i));
    }
  }

  if (length > a->length - idx) growBy(a, length - (a->length - idx));
  copynEleAt(a, idx, eleArray, length);
}

void Array_set(Array a, int64_t idx, void* ele) {
  Array_setn(a, idx, ele, 1, true);
}

void* Array_get(Array a, int64_t idx) {
  if (a == NULL) return NULL;
  idx = toRealIdx(a, idx);
  if (idx < 0) return NULL;
  return a->array + idx * a->elementSize;
}

void Array_getnCopy(Array a, int64_t idx, uint64_t n, void* eleArr) {
  if (a == NULL) exitWithError("@Array_getnCopy Array instance can't be NULL");
  idx = toRealIdx(a, idx);
  if (idx < 0) exitWithError("@Array_getnCopy index out of bounds");
  if (idx + n > a->length) exitWithError("@Array_getnCopy number of elements out of bounds");
  sysMemcpy(eleArr, a->array + idx * a->elementSize, n * a->elementSize);
}

void Array_clear(Array a) {
  if (a == NULL) exitWithError("@Array_clear Array instance can't be NULL");
  if (a->freeEleFn != NULL) {
    for (int32_t i = 0; i < a->length; ++i) {
      a->freeEleFn(Array_get(a, i));
    }
  }
  a->length = 0;
}

uint64_t Array_getLen(Array a) {
  if (a == NULL) exitWithError("@Array_getLen Array instance can't be NULL");
  return a->length;
}

void Array_printInfo(Array a) {
  if (a == NULL) exitWithError("@Array_printInfo Array instance can't be NULL");
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

void Array_concat(Array dst, Array src) {
  if (dst == NULL || src == NULL) exitWithError("@Array_concat Array instances can't be NULL");
  if (dst->elementSize != src->elementSize) {
    exitWithError("@Array_concat both Array instances should be of same element type");
  }

  uint64_t neededCapacity = dst->length + src->length;
  if (dst->capacity < neededCapacity) growTo(dst, neededCapacity);
  copynEleAt(dst, dst->length, src->array, src->length);
  dst->length += src->length;
}

const void* Array_getVanillaArray(Array a) {
  if (a == NULL) exitWithError("@Array_getVanillaArray Array instance can't be NULL");
  return a->array;
}

void Array_copyVanillaArrayInto(Array a, void* eleArr) {
  Array_getnCopy(a, 0, a->length, eleArr);
}

void* Array_getVanillaArrayCopy(Array a) {
  if (a == NULL) exitWithError("@Array_getVanillaArrayCopy Array instance can't be NULL");
  void* array = sysMalloc(a->length * a->elementSize);
  Array_copyVanillaArrayInto(a, array);
  return array;
}

bool Array_equals(Array a1, Array a2) {
  if (a1 == NULL || a2 == NULL) exitWithError("@Array_equals Array instances can't be NULL");
  if (a1->elementSize != a2->elementSize) exitWithError("@Array_equals Arrays must be of same element type");
  if (a1->cmpEleFn == NULL && a2->cmpEleFn == NULL) {
    exitWithError("@Array_equals One of the arrays must've been initalized with compare function");
  }

  CompareEleFn cmp = (a1->cmpEleFn != NULL) ? a1->cmpEleFn : a2->cmpEleFn;
  bool equal = true;
  for (int32_t i = 0; i < a1->length && equal; ++i) {
    void* ele1 = Array_get(a1, i);
    void* ele2 = Array_get(a2, i);
    if (cmp(ele1, ele2) != 0) equal = false;
  }

  return equal;
}

int32_t Array_find(Array a, void* ele) {
  if (a == NULL) exitWithError("@Array_find Array instances can't be NULL");
  if (a->cmpEleFn == NULL) exitWithError("@Array_find Array must've been initalized with compare function");

  for (int32_t i = 0; i < a->length; ++i) {
    void* ele1 = Array_get(a, i);
    if (a->cmpEleFn(ele, ele1) == 0) return i;
  }

  return -1;
}

bool Array_has(Array a, void* ele) {
  return Array_find(a, ele) >= 0;
}

void Array_remove(Array a, int64_t idx) {
  if (a == NULL) exitWithError("@Array_remove Array instance can't be NULL");
  idx = toRealIdx(a, idx);
  if (idx < 0) exitWithError("@Array_remove index out of bounds");
  if (idx == a->length - 1) {
    Array_pop(a);
    return;
  }
  if (a->freeEleFn != NULL) a->freeEleFn(Array_get(a, idx));
  int32_t lenToCopy = a->length - (idx + 1);
  // Don't want to free the elements that will remain.
  Array_setn(a, idx, a->array + (idx + 1) * a->elementSize, lenToCopy, false);
  --a->length;
}

uint64_t Array_getEleSize(Array a) {
  if (a == NULL) exitWithError("@Array_getEleSize Array instance can't be NULL");
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
  void* aux = shittyRealloc(a->array, a->capacity * a->elementSize, newCapacity * a->elementSize);
  if (aux == NULL) exitWithError("@Array's internal func growTo realloc error");
  a->capacity = newCapacity;
  a->array = aux;
}

void growBy(Array a, uint64_t extraCapacity) {
  growTo(a, a->capacity + extraCapacity);
}

int64_t toRealIdx(Array a, int64_t idx) {
  // We need to first check if idx < 0 because if it's negative and we compare directly
  // against length (unsigned) then idx will be cast to unsigned and will be a huge number
  // almost certainly larger than length.
  if (idx < 0) {
    // Note: we negate idx because a->length is unsigned
    if (-idx > a->length) return -1;
    idx += a->length;
  } else if (idx >= a->length) return -1;
  return idx;
}