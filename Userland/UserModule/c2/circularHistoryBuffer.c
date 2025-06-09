#include <array.h>
#include <circularHistoryBuffer.h>
#include <stdlib.h>
#include <syscalls.h>
#include <utils.h>

typedef struct CircularHistoryBufferADT {
  uint64_t size;
  uint64_t readIdx;
  uint64_t writeIdx;
  uint64_t toReadBackwards;
  uint64_t elementSize;
  FreeEleFn freeEleFn;
  Array array;
} CircularHistoryBufferADT;

void copynEleAt(CircularHistoryBuffer cb, uint64_t idx, const void* eleArr, uint64_t n);
void increaseWriteIdx(CircularHistoryBuffer cb);
void decreaseWriteIdx(CircularHistoryBuffer cb);
void increaseReadIdx(CircularHistoryBuffer cb);
void decreaseReadIdx(CircularHistoryBuffer cb);

CircularHistoryBuffer CHB_initialize(uint64_t elementSize, uint64_t size, FreeEleFn freeEleFn, CompareEleFn cmpEleFn) {
  CircularHistoryBuffer cb = sysMalloc(sizeof(CircularHistoryBufferADT));
  if (cb == NULL) exitWithError("@CHB_initialize malloc error");
  cb->array = Array_initialize(elementSize, size, freeEleFn, cmpEleFn);
  cb->size = size;
  cb->readIdx = 0;
  cb->writeIdx = 0;
  cb->toReadBackwards = 0;
  cb->elementSize = elementSize;
  cb->freeEleFn = freeEleFn;

  return cb;
}

void CHB_push(CircularHistoryBuffer cb, void* ele) {
  if (cb == NULL) exitWithError("@CHB_writeNext CHB instance can't be NULL");
  if (Array_getLen(cb->array) < cb->size) Array_push(cb->array, ele);
  else Array_set(cb->array, cb->writeIdx, ele);
  increaseWriteIdx(cb);
  CHB_readRest(cb);
}

void* CHB_readNext(CircularHistoryBuffer cb) {
  if (cb == NULL) exitWithError("@CHB_readNext CHB instance can't be NULL");
  if (cb->toReadBackwards + 1 >= Array_getLen(cb->array)) return NULL;
  increaseReadIdx(cb);
  ++cb->toReadBackwards;
  return Array_get(cb->array, cb->readIdx);
}

void* CHB_readPrev(CircularHistoryBuffer cb) {
  if (cb == NULL) exitWithError("@CHB_readPrev CHB instance can't be NULL");
  if (cb->toReadBackwards == 0) return NULL;
  decreaseReadIdx(cb);
  --cb->toReadBackwards;
  return Array_get(cb->array, cb->readIdx);
}

void CHB_readRest(CircularHistoryBuffer cb) {
  if (cb == NULL) exitWithError("@CHB_readRest CHB instance can't be NULL");
  cb->readIdx = cb->writeIdx;
  cb->toReadBackwards = Array_getLen(cb->array);
}

// Move 3
// og seq  : 7, 6, 5, 4, 3, 2, 1
// expected: 3, 7, 6, 5, 4, 2, 1
//     writeIdx
//         v
// [ 6, 7, 1, 2, 3, 4, 5 ]
// Move from writeIdx to idx-1 one position forward
// [ 6, 7, 1, 1, 2, 4, 5 ]
//            v
// [ 6, 7, 3, 1, 2, 4, 5 ]

// Move 5
// og seq  : 7, 6, 5, 4, 3, 2, 1
// expected: 5, 7, 6, 4, 3, 2, 1
//           writeIdx
//               v
// [ 4, 5, 6, 7, 1, 2, 3 ]
// Move from idx+1 to writeIdx-1 one position back
// [ 4, 6, 7, 7, 1, 2, 3 ]
// [ 4, 6, 7, 5, 1, 2, 3 ]

void CHB_moveToFrontOrPush(CircularHistoryBuffer cb, void* ele) {
  int32_t idx = Array_find(cb->array, ele);
  if (idx >= 0 && idx != cb->writeIdx) {
    cb->freeEleFn(Array_get(cb->array, idx));
    if (idx < cb->writeIdx) {
      const void* eleArr = Array_getVanillaArray(cb->array);
      Array_setn(cb->array, idx, eleArr + (idx + 1) * cb->elementSize, cb->writeIdx - (idx + 1), false);
      Array_setn(cb->array, cb->writeIdx - 1, ele, 1, false);
    } else if (idx > cb->writeIdx) {
      uint64_t len = idx - cb->writeIdx;
      const void* eleArr[len * cb->elementSize];
      Array_getnCopy(cb->array, cb->writeIdx, len, eleArr);
      Array_setn(cb->array, cb->writeIdx + 1, eleArr, len, false);
      Array_setn(cb->array, cb->writeIdx, ele, 1, false);
      increaseWriteIdx(cb);
    }
    CHB_readRest(cb);
  } else {
    CHB_push(cb, ele);
  }
}

uint64_t CHB_getLen(CircularHistoryBuffer cb) {
  if (cb == NULL) exitWithError("@CHB_getLen CHB instance can't be NULL");
  return cb->toReadBackwards;
}

uint64_t CHB_getSize(CircularHistoryBuffer cb) {
  if (cb == NULL) exitWithError("@CHB_getSize CHB instance can't be NULL");
  return cb->size;
}

void CHB_free(CircularHistoryBuffer cb) {
  if (cb == NULL) exitWithError("@CHB_free CHB instance can't be NULL");
  Array_free(cb->array);
  sysFree(cb);
}

bool CHB_has(CircularHistoryBuffer cb, void* ele) {
  if (cb == NULL) exitWithError("@CHB_has CHB instance can't be NULL");
  return Array_has(cb->array, ele);
}

/////////////// Internal functions ///////////////

uint64_t getIncreasedIdxBy(CircularHistoryBuffer cb, uint64_t idx, uint64_t val) {
  return (idx + val) % cb->size;
}

uint64_t getDecreasedIdxBy(CircularHistoryBuffer cb, uint64_t idx, uint64_t val) {
  val %= Array_getLen(cb->array) + 1;
  if (idx < val) idx += Array_getLen(cb->array) - val;
  else idx -= val;
  return idx;
}

void increaseWriteIdx(CircularHistoryBuffer cb) {
  cb->writeIdx = getIncreasedIdxBy(cb, cb->writeIdx, 1);
}

void decreaseWriteIdx(CircularHistoryBuffer cb) {
  cb->writeIdx = getDecreasedIdxBy(cb, cb->writeIdx, 1);
}

void increaseReadIdx(CircularHistoryBuffer cb) {
  cb->readIdx = getIncreasedIdxBy(cb, cb->readIdx, 1);
}

void decreaseReadIdx(CircularHistoryBuffer cb) {
  cb->readIdx = getDecreasedIdxBy(cb, cb->readIdx, 1);
}