//#include <array.h>
#include <memory.h>
#include "../include/pipes.h"
#include <semaphores.h>
#include <stdbool.h>
#include <stdint.h>
#include <utils.h>

#define INITIAL_CAPACITY 20

static Array pipeArray;
static Array freedPositions;

void freePipe(Pipe** p) {
  globalFree(*p);
}

static Pipe stdinPipe;
static Pipe stderrPipe;

void initPipes() {
  pipeArray = initArray(sizeof(Pipe*), INITIAL_CAPACITY, (ElementDestructor)freePipe);
  freedPositions = initArray(sizeof(int32_t), INITIAL_CAPACITY, NULL);
  // STDOUT --> Not used but I need to occupy this index anyways.
  createPipe();
  stdinPipe = **(Pipe**)getAtArrayIdx(pipeArray, createPipe());
  stderrPipe = **(Pipe**)getAtArrayIdx(pipeArray, createPipe());
}

int64_t createPipe() {
  Pipe* p = globalMalloc(sizeof(Pipe));
  p->mutex = initSem(1);
  p->full = initSem(0);
  p->empty = initSem(BUFFER_SIZE);
  p->producingIndex = 0;
  p->consumingIndex = 0;
  p->deleted = false;
  // p->readerPcb = NULL;
  // p->writerPcb = NULL;
  int32_t freeToUse;
  if (popAndGetFromArray(freedPositions, &freeToUse)) {
    // I could use get and save having to allocate a new pipe.
    setAtArrayIdx(pipeArray, freeToUse, &p);
    return freeToUse;
  } else {
    return pushToArray(pipeArray, &p);
  }
}

Pipe* fetchPipe(int32_t pipeId) {
  if (pipeId < 0) return NULL;
  Pipe** pPtr = getAtArrayIdx(pipeArray, pipeId);
  if (pPtr == NULL) return NULL;
  Pipe* p = *pPtr;
  if (p->deleted) return NULL;
  return p;
}

int64_t readFromPipe(int32_t pipeId, char* buf, int32_t len) {
  if (len <= 0) return 0;
  Pipe* p = fetchPipe(pipeId);
  if (p == NULL) return -1;
  int64_t pos = 0;
  bool reachedEnd = false;
  do {
    waitSemaphore(p->full);
    waitSemaphore(p->mutex);
    buf[pos++] = p->buffer[p->consumingIndex];
    p->consumingIndex = (p->consumingIndex + 1) % BUFFER_SIZE;
    if (p->consumingIndex == p->producingIndex) reachedEnd = true;
    postSemaphore(p->mutex);
    postSemaphore(p->empty);
  } while (pos < len && !reachedEnd);

  return pos;
}

int64_t writeToPipe(int32_t pipeId, const char* buf, int32_t len) {
  if (pipeId == STDIN) return -1;
  if (len <= 0) return 0;
  Pipe* p = fetchPipe(pipeId);
  if (p == NULL) return -1;
  int64_t pos = 0;
  while (pos < len) {
    waitSemaphore(p->empty);
    waitSemaphore(p->mutex);
    p->buffer[p->producingIndex] = buf[pos++];
    p->producingIndex = (p->producingIndex + 1) % BUFFER_SIZE;
    postSemaphore(p->mutex);
    postSemaphore(p->full);
  }
  return pos;
}

void writeStdin(char c) {
  stdinPipe.buffer[stdinPipe.producingIndex] = c;
  stdinPipe.producingIndex = (stdinPipe.producingIndex + 1) % BUFFER_SIZE;
  postSemaphore(stdinPipe.full);
}

int64_t readStdin(char* buf, int32_t len) {
  if (len <= 0) return 0;
  int64_t pos = 0;
  bool reachedEnd = false;
  do {
    waitSemaphore(stdinPipe.full);
    waitSemaphore(stdinPipe.mutex);
    buf[pos++] = stdinPipe.buffer[stdinPipe.consumingIndex];
    stdinPipe.consumingIndex = (stdinPipe.consumingIndex + 1) % BUFFER_SIZE;
    if (stdinPipe.consumingIndex == stdinPipe.producingIndex) reachedEnd = true;
    postSemaphore(stdinPipe.mutex);
  } while (pos < len && !reachedEnd);

  return pos;
}

bool deletePipe(int32_t pipeId) {
  if (0 <= pipeId && pipeId <= 2) return false;
  Pipe* p = fetchPipe(pipeId);
  if (p == NULL) return false;
  p->deleted = true;
  pushToArray(freedPositions, &pipeId);
  destroySemaphore(p->mutex);
  destroySemaphore(p->full);
  destroySemaphore(p->empty);
  // setAtArrayIdx will do the globalFree of the pipe itself when it overrides this position.
  return true;
}