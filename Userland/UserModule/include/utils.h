#ifndef UTILS_H
#define UTILS_H

#include <keyboard.h>
#include <stdbool.h>

bool justCtrlMod(KeyStruct* key);
void* realloc(void* ptr, uint64_t oldSize, uint64_t newSize);
void exitWithError(const char* msg);
int32_t strncpy(char* dst, const char* src, int32_t max);
int32_t strcpy(char* dst, const char* src);

#endif