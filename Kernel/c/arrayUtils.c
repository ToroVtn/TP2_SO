#include "../include/arrayUtils.h"
#include <lib.h>
#include <memory.h>
#include <stdint.h>
#include <syscalls.h>
#include <utils.h>

typedef struct ArrayCDT { 
  uint8_t* array;
  uint64_t elemSize;
  uint64_t capacity;
  uint64_t len;
  ElementDestructor elementDestructor;
} ArrayCDT;

void copynElemAt(Array arr, uint64_t idx, const void* elemArr, uint64_t n);
void copyElemAt(Array arr, uint64_t idx, const void* elem);
bool growTo(Array arr, uint64_t newCapacity);
bool growBy(Array arr, uint64_t extraCapacity);

void* initArray(uint64_t elemSize, uint64_t initialCapacity, ElementDestructor elementDestructor) {
  if (elemSize == 0) {
    return NULL;
  }
  ArrayCDT* arr = malloc(sizeof(ArrayCDT));
  if (arr == NULL) {
    return NULL;
  }
  arr->capacity = initialCapacity ? initialCapacity : 1;
  arr->array = malloc(arr->capacity * elemSize);
  if (arr->array == NULL) {
    free(arr);
    return NULL;
  }
  arr->len = 0;
  arr->elemSize = elemSize;
  arr->elementDestructor = elementDestructor;
  return arr;
}

bool freeArray(Array arr) {
  if (arr == NULL) {
    return false;
  }

  if (arr->elementDestructor != NULL) {
    for (int32_t i = 0; i < arr->len; ++i) arr->elementDestructor(getAtArrayIdx(arr, i));
  }
  free(arr->array);
  free(arr);
  return true;
}

int64_t pushToArray(Array arr, const void* elem) {
  if (arr == NULL || elem == NULL) { 
    return -1;
  }

  if (arr->len >= arr->capacity) growBy(arr, arr->capacity);

  copyElemAt(arr, arr->len++, elem);
  return arr->len - 1;
}

bool popAndGetFromArray(Array arr, void* elem) {
  if (arr == NULL || arr->len == 0) {
    return false;
  }

  void* elemToPop = getAtArrayIdx(arr, -1);
  memcpy(elem, elemToPop, arr->elemSize);
  --arr->len;
  return true;
}

bool popArray(Array arr) {
  if (arr == NULL || arr->len == 0){
    return false;
  }
  if (arr->elementDestructor != NULL){
    arr->elementDestructor(getAtArrayIdx(arr, -1));
  } 
  --arr->len;
  return true;
}

bool arrayCopyInto(Array arr, int64_t idx, const void* elemArray, uint64_t len) {
  if (arr == NULL) {
    return false;
  }
  if (idx < 0) {
    if (-idx > arr->len) {
        return false;
    } 
    idx += arr->len;
  } else if (idx >= arr->len) {
    return NULL;
  }
  if (arr->elementDestructor != NULL) {
    for (int32_t i = idx; i < idx + len; ++i) {
      arr->elementDestructor(getAtArrayIdx(arr, i));
    }
  }
  if (len > arr->len - idx) growBy(arr, len - (arr->len - idx));
  copynElemAt(arr, idx, elemArray, len);
  return true;
}

bool setAtArrayIdx(Array arr, int64_t idx, void* elem) {
  return arrayCopyInto(arr, idx, elem, 1);
}

void* getAtArrayIdx(Array arr, int64_t idx) {
  if (arr == NULL) {
    return NULL;
  }
  if (idx < 0) {
    if (-idx > arr->len) {
        return NULL;
    }
    idx += arr->len;
  } else if (idx >= arr->len){
        return NULL;
  } 
  return arr->array + idx * arr->elemSize;
}

bool emptyArray(Array arr) {
  if (arr == NULL){
    return false;
  } 
  if (arr->elementDestructor != NULL) {
    for (int32_t i = 0; i < arr->len; ++i) {
      arr->elementDestructor(getAtArrayIdx(arr, i));
    }
  }
  arr->len = 0;
  return true;
}

uint64_t arrayLen(Array arr) {
  if (arr == NULL) {
    return -1;
  }
  return arr->len;
}

Array CArrayToMyAarray(const void* array, uint64_t len, uint64_t elemSize, ElementDestructor freeFn) {
  Array arr = initArray(elemSize, len, freeFn);
  if (arr == NULL) {
    return NULL;
  }
  arr->len = len;
  copynElemAt(arr, 0, array, len);
  return arr;
}

bool concatArray(Array destiny, Array source) {
  if (destiny == NULL || source == NULL || destiny->elemSize != source->elemSize) {
    return false;
  }
  uint64_t neededCapacity = destiny->len + source->len;
  if (destiny->capacity < neededCapacity) {
    growTo(destiny, neededCapacity);
  }
  copynElemAt(destiny, destiny->len, source->array, source->len);
  destiny->len += source->len;
  return true;
}

const void* arrayData(Array arr) {
  if (arr == NULL) {
    return NULL;
  }
  return arr->array;
}

void* arrayCopyTo(Array arr, void* array) {
  if (arr == NULL) {
    return NULL;
  }
  memcpy(array, arr->array, arr->len * arr->elemSize);
  return array;
}

void* arrayCloneAsCArray(Array arr) {
  if (arr == NULL) {
    return NULL;
  }
  void* array = malloc(arr->len * arr->elemSize);
  arrayCopyTo(arr, array);
  return array;
}



void copynElemAt(Array arr, uint64_t idx, const void* elemArr, uint64_t n) {
  memcpy(arr->array + arr->elemSize * idx, elemArr, arr->elemSize * n);
}

void copyElemAt(Array arr, uint64_t idx, const void* elem) {
  copynElemAt(arr, idx, elem, 1);
}

bool growTo(Array arr, uint64_t newCapacity) {
  void* aux = realloc(arr->array, arr->capacity * arr->elemSize, newCapacity * arr->elemSize);
  if (aux == NULL) {
    return false;
  } 
  arr->capacity = newCapacity;
  arr->array = aux;
  return true;
}

bool growBy(Array arr, uint64_t extraCapacity) {
  return growTo(arr, arr->capacity + extraCapacity);
}