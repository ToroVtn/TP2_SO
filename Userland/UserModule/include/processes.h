#ifndef PROCESSES_H
#define PROCESSES_H

#include <stdint.h>

#define MAX_NAME_LENGTH 50

typedef struct {
  uint32_t pid;
  uint8_t priority;
  const char* state;
  void* rsp;
  void* rbp;
  char name[MAX_NAME_LENGTH + 1];
  char* location;
} PCB;

#endif