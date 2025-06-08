#include <stdlib.h>
#include <syscalls.h>
#include <utils.h>

bool justCtrlMod(KeyStruct* key) {
  return key->md.ctrlPressed && !key->md.altPressed && !key->md.leftShiftPressed && !key->md.rightShiftPressed;
}

void* realloc(void* ptr, uint64_t oldSize, uint64_t newSize) {
  void* mem = sysMalloc(newSize);
  if (mem == NULL) return NULL;
  sysMemcpy(mem, ptr, oldSize);
  sysFree(ptr);
  return mem;
}

void exitWithError(const char* msg) {
  printf("%s\n", msg);
  sysExit(1);
}

int32_t strncpy(char* dst, const char* src, int32_t max) {
  if (max < 0) return -1;
  int32_t i = 0;
  for (; i < max && src[i] != 0; ++i) dst[i] = src[i];
  dst[i] = 0;
  return i;
}

int32_t strcpy(char* dst, const char* src) {
  int32_t i = 0;
  for (; src[i] != 0; ++i) dst[i] = src[i];
  dst[i] = 0;
  return i;
}