#include <utils.h>

int strncpy(char* dst, char* src, int max) {
  int i = 0;
  if (max > 0) for (; i < max && src[i] != 0; ++i) dst[i] = src[i];
  else for (; src[i] != 0; ++i) dst[i] = src[i];
  dst[i] = 0;
  return i;
}

int strcpy(char* dst, char* src) {
  return strncpy(dst, src, 0);
}