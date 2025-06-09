#include <utils.h>

int32_t strncpy(char* dst, char* src, int32_t max) {
  if (max < 0) return -1;
  int32_t i = 0;
  for (; i < max && src[i] != 0; ++i) dst[i] = src[i];
  dst[i] = 0;
  return i;
}

int32_t strcpy(char* dst, char* src) {
  int32_t i = 0;
  for (; src[i] != 0; ++i) dst[i] = src[i];
  dst[i] = 0;
  return i;
}

unsigned int strlen(char* s) {
    int len = 0;
    while (s[len++] != 0);
    return len - 1;
}
int strcmp(  char* s1,   char* s2) {
    int i = 0;
    for (; s1[i] != 0 && s2[i] != 0; ++i) {
        if (s1[i] < s2[i]) return -1;
        else if (s1[i] > s2[i]) return 1;
    }
    if (s1[i] != 0) return 1;
    else if (s2[i] != 0) return -1;
    else return 0;
}

char* strcat(char* dest,   char* src) {
    char* originalDest = dest;
    
   
    while (*dest != '\0') {
        dest++;
    }
    
    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }
    
   
    *dest = '\0';
    
    return originalDest;
}

uint32_t uintToBase(uint64_t value, char* buffer, uint32_t base) {
  char* p = buffer;
  uint32_t digits = 0;

  do {
    uint32_t remainder = value % base;
    *p++ = (remainder < 10) ? remainder + '0' : remainder + 'A' - 10;
    digits++;
  } while (value /= base);

  *p = 0;

  char* p1 = buffer;
  char* p2;
  p2 = p - 1;
  while (p1 < p2) {
    char tmp = *p1;
    *p1 = *p2;
    *p2 = tmp;
    p1++;
    p2--;
  }

  return digits;
}