#include <arrayUtils.h>
#include <memory.h>
#include <pipes.h>
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
  

  createPipe();
  
  
  int32_t stdinPipeId = createPipe();
  stdinPipe = **(Pipe**)getAtArrayIdx(pipeArray, stdinPipeId);
  
  
  int32_t stderrPipeId = createPipe();
  stderrPipe = **(Pipe**)getAtArrayIdx(pipeArray, stderrPipeId);
  
  
  for (int i = 0; i < BUFFER_SIZE; i++) {
    stdinPipe.buffer[i] = 0;
    stderrPipe.buffer[i] = 0;
  }
  
  
  destroySemaphore(stdinPipe.mutex);
  destroySemaphore(stdinPipe.written);
  destroySemaphore(stdinPipe.empty);
  
  stdinPipe.mutex = initSem(1);        
  stdinPipe.written = initSem(0);      
  stdinPipe.empty = initSem(BUFFER_SIZE); 
}

int64_t createPipe() {
  Pipe* p = globalMalloc(sizeof(Pipe));
  p->mutex = initSem(1);
  p->written = initSem(0);
  p->empty = initSem(BUFFER_SIZE);
  p->producingIndex = 0;
  p->consumingIndex = 0;
  p->deleted = false;
  
  int32_t freeToUse;
  if (popAndGetFromArray(freedPositions, &freeToUse)) {
   
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
    waitSemaphore(p->written);
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
    postSemaphore(p->written);
  }
  return pos;
}

void writeStdin(char c) {
  waitSemaphore(stdinPipe.empty);  
  waitSemaphore(stdinPipe.mutex);  
  stdinPipe.buffer[stdinPipe.producingIndex] = c;
  stdinPipe.producingIndex = (stdinPipe.producingIndex + 1) % BUFFER_SIZE;
  postSemaphore(stdinPipe.mutex); 
  postSemaphore(stdinPipe.written);  
}

int64_t readStdin(char* buf, int32_t len) {
  if (len <= 0) return 0;
  
  // IMPORTANT: Remove this check that's causing the race condition
  // if (stdinPipe.consumingIndex == stdinPipe.producingIndex) {
  //   return 0; 
  // }
  
  // Always wait for data to be available - this will cause lag but prevent the loop
  waitSemaphore(stdinPipe.written);
  waitSemaphore(stdinPipe.mutex);
  
  // Get the character
  *buf = stdinPipe.buffer[stdinPipe.consumingIndex];
  
  // Clear the buffer position
  stdinPipe.buffer[stdinPipe.consumingIndex] = 0;
  
  // Advance the index
  stdinPipe.consumingIndex = (stdinPipe.consumingIndex + 1) % BUFFER_SIZE;
  
  // Release the mutex
  postSemaphore(stdinPipe.mutex);
  postSemaphore(stdinPipe.empty);
  
  return 1;
}

bool deletePipe(int32_t pipeId) {
  if (0 <= pipeId && pipeId <= 2) return false;
  Pipe* p = fetchPipe(pipeId);
  if (p == NULL) return false;
  p->deleted = true;
  pushToArray(freedPositions, &pipeId);
  destroySemaphore(p->mutex);
  destroySemaphore(p->written);
  destroySemaphore(p->empty);
  
  return true;
}