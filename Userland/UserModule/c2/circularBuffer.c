#include <circularBuffer.h>

void incCircularIdxBy(int* idx, int val, int len) {
  *idx = (*idx + val) % len;
}

void incCircularIdx(int* idx, int len) {
  incCircularIdxBy(idx, 1, len);
}

void decCircularIdxBy(int* idx, int val, int len) {
  *idx = *idx - val;
  if (*idx < 0) *idx = len + *idx;
}

void decCircularIdx(int* idx, int len) {
  decCircularIdxBy(idx, 1, len);
}