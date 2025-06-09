#ifndef PIPES_H
#define PIPES_H

#include <stdint.h>

typedef struct {
  int32_t write;
  int32_t read;
  int32_t err;
} Pipe;

#endif