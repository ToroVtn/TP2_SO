#include <utils.h>

int strcpy(char* dst, char* src) {
  return strncpy(dst, src, 0);
}

int strncpy(char* dst, char* src, int max) {
  int i = 0;
  if (max > 0) for (; i < max && src[i] != 0; ++i) dst[i] = src[i];
  else for (; src[i] != 0; ++i) dst[i] = src[i];
  dst[i] = 0;
  return i;
}

unsigned int s_strlen(char* s) {
    int len = 0;
    while (s[len++] != 0);
    return len - 1;
}
int s_strcmp(const char* s1, const char* s2) {
    int i = 0;
    for (; s1[i] != 0 && s2[i] != 0; ++i) {
        if (s1[i] < s2[i]) return -1;
        else if (s1[i] > s2[i]) return 1;
    }
    if (s1[i] != 0) return 1;
    else if (s2[i] != 0) return -1;
    else return 0;
}
