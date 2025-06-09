
#include <array.h>
#include <lib.h>
#include <memoryManager.h>
#include <stdint.h>
#include <syscalls.h>
#include <utils.h>

typedef struct ArrayCDT {
  // Use uint8_t* instead of void* because void* can't be used in arithmetic operations.
  uint8_t* array;
  uint64_t elementSize;
  uint64_t capacity;
  uint64_t length;
  FreeEleFn freeEleFn;
} ArrayCDT;

void copynEleAt(Array a, uint64_t idx, const void* eleArr, uint64_t n);
void copyEleAt(Array a, uint64_t idx, const void* ele);
bool growTo(Array a, uint64_t newCapacity);
bool growBy(Array a, uint64_t extraCapacity);

void* Array_initialize(uint64_t elementSize, uint64_t initialCapacity, FreeEleFn freeEleFn) {
  if (elementSize == 0) return NULL;
  ArrayCDT* a = globalMalloc(sizeof(ArrayCDT));
  if (a == NULL) return NULL;
  a->capacity = initialCapacity ? initialCapacity : 1;
  a->array = globalMalloc(a->capacity * elementSize);
  if (a->array == NULL) {
    globalFree(a);
    return NULL;
  }
  a->length = 0;
  a->elementSize = elementSize;
  a->freeEleFn = freeEleFn;
  return a;
}

bool Array_free(Array a) {
  if (a == NULL) return false;

  if (a->freeEleFn != NULL) {
    for (int32_t i = 0; i < a->length; ++i) a->freeEleFn(Array_get(a, i));
  }
  globalFree(a->array);
  globalFree(a);
  return true;
}

int64_t Array_push(Array a, const void* ele) {
  if (a == NULL || ele == NULL) return -1;

  if (a->length >= a->capacity) growBy(a, a->capacity);

  copyEleAt(a, a->length++, ele);
  return a->length - 1;
}

bool Array_popGetEle(Array a, void* ele) {
  if (a == NULL || a->length == 0) return false;

  void* eleToPop = Array_get(a, -1);
  memcpy(ele, eleToPop, a->elementSize);
  // This doesn't make sense if I want to get the element back...
  // if (a->freeEleFn != NULL) a->freeEleFn(eleToPop);
  --a->length;
  return true;
}

bool Array_pop(Array a) {
  if (a == NULL || a->length == 0) return false;
  if (a->freeEleFn != NULL) a->freeEleFn(Array_get(a, -1));
  --a->length;
  return true;
}

bool Array_setn(Array a, int64_t idx, const void* eleArray, uint64_t length) {
  if (a == NULL) return false;
  if (idx < 0) {
    if (-idx > a->length) return false;
    idx += a->length;
  } else if (idx >= a->length) return NULL;
  if (a->freeEleFn != NULL) {
    for (int32_t i = idx; i < idx + length; ++i) {
      a->freeEleFn(Array_get(a, i));
    }
  }
  if (length > a->length - idx) growBy(a, length - (a->length - idx));
  copynEleAt(a, idx, eleArray, length);
  return true;
}

bool Array_set(Array a, int64_t idx, void* ele) {
  return Array_setn(a, idx, ele, 1);
}

void* Array_get(Array a, int64_t idx) {
  if (a == NULL) return NULL;
  if (idx < 0) {
    // Note: we negate idx because a->length is unsigned
    if (-idx > a->length) return NULL;
    idx += a->length;
  } else if (idx >= a->length) return NULL;
  return a->array + idx * a->elementSize;
}

bool Array_clear(Array a) {
  if (a == NULL) return false;
  if (a->freeEleFn != NULL) {
    for (int32_t i = 0; i < a->length; ++i) {
      a->freeEleFn(Array_get(a, i));
    }
  }
  a->length = 0;
  return true;
}

uint64_t Array_getLen(Array a) {
  if (a == NULL) return -1;
  return a->length;
}

Array Array_fromVanillaArray(const void* array, uint64_t length, uint64_t elementSize, FreeEleFn freeFn) {
  Array a = Array_initialize(elementSize, length, freeFn);
  if (a == NULL) return NULL;
  a->length = length;
  copynEleAt(a, 0, array, length);
  return a;
}

bool Array_concat(Array dst, Array src) {
  if (dst == NULL || src == NULL || dst->elementSize != src->elementSize) return false;
  uint64_t neededCapacity = dst->length + src->length;
  if (dst->capacity < neededCapacity) growTo(dst, neededCapacity);
  copynEleAt(dst, dst->length, src->array, src->length);
  dst->length += src->length;
  return true;
}

const void* Array_getVanillaArray(Array a) {
  if (a == NULL) return NULL;
  return a->array;
}

void* Array_copyVanillaArrayInto(Array a, void* array) {
  if (a == NULL) return NULL;
  memcpy(array, a->array, a->length * a->elementSize);
  return array;
}

void* Array_getVanillaArrayCopy(Array a) {
  if (a == NULL) return NULL;
  void* array = globalMalloc(a->length * a->elementSize);
  Array_copyVanillaArrayInto(a, array);
  return array;
}

//////////////////////////// Internal Functions ////////////////////////////

void copynEleAt(Array a, uint64_t idx, const void* eleArr, uint64_t n) {
  memcpy(a->array + a->elementSize * idx, eleArr, a->elementSize * n);
}

void copyEleAt(Array a, uint64_t idx, const void* ele) {
  copynEleAt(a, idx, ele, 1);
}

bool growTo(Array a, uint64_t newCapacity) {
  void* aux = shittyRealloc(a->array, a->capacity * a->elementSize, newCapacity * a->elementSize);
  if (aux == NULL) return false;
  a->capacity = newCapacity;
  a->array = aux;
  return true;
}

bool growBy(Array a, uint64_t extraCapacity) {
  return growTo(a, a->capacity + extraCapacity);
}
