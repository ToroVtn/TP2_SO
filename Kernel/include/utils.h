#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

int32_t strncpy(char* dst,   char* src, int max);
int32_t strcpy(char* dst,   char* src);
unsigned int strlen(  char* s);
int strcmp(  char* s1,   char* s2);
char* strcat(char* dest,   char* src);
uint32_t uintToBase(uint64_t value, char* buffer, uint32_t base);

#endif