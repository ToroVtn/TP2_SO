#include <arrayUtils.h>
#include <circularHistoryBuffer.h>
#include <stdlib.h>
#include <syscalls.h>
#include <utils.h>

typedef struct CircularHistoryBufferADT {
  uint64_t size;
  uint64_t readIdx;
  uint64_t producingIndex;
  uint64_t toReadBackwards;
  uint64_t elementSize;
  ElementDestructor freeElemFn;
  Array array;
} CircularHistoryBufferADT;

void copynElemAt(CircularHistoryBuffer cb, uint64_t idx, const void* eleArr, uint64_t n);
void increaseWriteIdx(CircularHistoryBuffer cb);
void decreaseWriteIdx(CircularHistoryBuffer cb);
void increaseReadIdx(CircularHistoryBuffer cb);
void decreaseReadIdx(CircularHistoryBuffer cb);

CircularHistoryBuffer initCHB(uint64_t elementSize, uint64_t size, ElementDestructor freeElemFn, CompareEleFn cmpEleFn) {
  CircularHistoryBuffer cb = sysMalloc(sizeof(CircularHistoryBufferADT));
  if (cb == NULL) exitWithError("@initCHB malloc error");
  cb->array = initArray(elementSize, size, freeElemFn, cmpEleFn);
  cb->size = size;
  cb->readIdx = 0;
  cb->producingIndex = 0;
  cb->toReadBackwards = 0;
  cb->elementSize = elementSize;
  cb->freeElemFn = freeElemFn;

  return cb;
}

void pushToCHB(CircularHistoryBuffer cb, void* ele) {
  if (cb == NULL) exitWithError("@CHB_writeNext CHB instance can't be NULL");
  if (arrayLen(cb->array) < cb->size) pushToArray(cb->array, ele);
  else setAtArrayIdx(cb->array, cb->producingIndex, ele);
  increaseWriteIdx(cb);
  readRestFromCHB(cb);
}

void* readNextFromCHB(CircularHistoryBuffer cb) {
  if (cb == NULL) exitWithError("@readNextFromCHB CHB instance can't be NULL");
  if (cb->toReadBackwards + 1 >= arrayLen(cb->array)) return NULL;
  increaseReadIdx(cb);
  ++cb->toReadBackwards;
  return getAtArrayIdx(cb->array, cb->readIdx);
}

void* readPrevFromCHB(CircularHistoryBuffer cb) {
  if (cb == NULL) exitWithError("@readPrevFromCHB CHB instance can't be NULL");
  if (cb->toReadBackwards == 0) return NULL;
  decreaseReadIdx(cb);
  --cb->toReadBackwards;
  return getAtArrayIdx(cb->array, cb->readIdx);
}

void readRestFromCHB(CircularHistoryBuffer cb) {
  if (cb == NULL) exitWithError("@readRestFromCHB CHB instance can't be NULL");
  cb->readIdx = cb->producingIndex;
  cb->toReadBackwards = arrayLen(cb->array);
}


void moveTofrontOrPushCHB(CircularHistoryBuffer cb, void* ele) {
  int32_t idx = findArray(cb->array, ele);
  if (idx >= 0 && idx != cb->producingIndex) {
    cb->freeElemFn(getAtArrayIdx(cb->array, idx));
    if (idx < cb->producingIndex) {
      const void* eleArr = arrayData(cb->array);
      arrayCopyInto(cb->array, idx, eleArr + (idx + 1) * cb->elementSize, cb->producingIndex - (idx + 1), false);
      arrayCopyInto(cb->array, cb->producingIndex - 1, ele, 1, false);
    } else if (idx > cb->producingIndex) {
      uint64_t len = idx - cb->producingIndex;
      const void* eleArr[len * cb->elementSize];
      copyElemAt(cb->array, cb->producingIndex, len, eleArr);
      arrayCopyInto(cb->array, cb->producingIndex + 1, eleArr, len, false);
      arrayCopyInto(cb->array, cb->producingIndex, ele, 1, false);
      increaseWriteIdx(cb);
    }
    readRestFromCHB(cb);
  } else {
    pushToCHB(cb, ele);
  }
}

uint64_t getCHBLen(CircularHistoryBuffer cb) {
  if (cb == NULL) exitWithError("@getCHBLen CHB instance can't be NULL");
  return cb->toReadBackwards;
}

uint64_t getCHBSize(CircularHistoryBuffer cb) {
  if (cb == NULL) exitWithError("@getCHBSize CHB instance can't be NULL");
  return cb->size;
}

void freeCHB(CircularHistoryBuffer cb) {
  if (cb == NULL) exitWithError("@freeCHB CHB instance can't be NULL");
  freeArray(cb->array);
  sysFree(cb);
}

bool containsCHB(CircularHistoryBuffer cb, void* ele) {
  if (cb == NULL) exitWithError("@containsCHB CHB instance can't be NULL");
  return hasArray(cb->array, ele);
}



uint64_t getIncreasedIdxBy(CircularHistoryBuffer cb, uint64_t idx, uint64_t val) {
  return (idx + val) % cb->size;
}

uint64_t getDecreasedIdxBy(CircularHistoryBuffer cb, uint64_t idx, uint64_t val) {
  val %= arrayLen(cb->array) + 1;
  if (idx < val) idx += arrayLen(cb->array) - val;
  else idx -= val;
  return idx;
}

void increaseWriteIdx(CircularHistoryBuffer cb) {
  cb->producingIndex = getIncreasedIdxBy(cb, cb->producingIndex, 1);
}

void decreaseWriteIdx(CircularHistoryBuffer cb) {
  cb->producingIndex = getDecreasedIdxBy(cb, cb->producingIndex, 1);
}

void increaseReadIdx(CircularHistoryBuffer cb) {
  cb->readIdx = getIncreasedIdxBy(cb, cb->readIdx, 1);
}

void decreaseReadIdx(CircularHistoryBuffer cb) {
  cb->readIdx = getDecreasedIdxBy(cb, cb->readIdx, 1);
}