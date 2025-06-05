#ifndef CICRULAR_BUFFER_H
#define CICRULAR_BUFFER_H

void incCircularIdxBy(int* idx, int val, int max);
void incCircularIdx(int* idx, int max);
void decCircularIdxBy(int* idx, int val, int max);
void decCircularIdx(int* idx, int max);

#endif