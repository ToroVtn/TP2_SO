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
  
  // Create pipes with explicit initialization
  // STDOUT --> Not used but I need to occupy this index anyways.
  createPipe();
  
  // Create and initialize stdinPipe
  int32_t stdinPipeId = createPipe();
  stdinPipe = **(Pipe**)getAtArrayIdx(pipeArray, stdinPipeId);
  
  // Create and initialize stderrPipe
  int32_t stderrPipeId = createPipe();
  stderrPipe = **(Pipe**)getAtArrayIdx(pipeArray, stderrPipeId);
  
  // Initialize all buffer elements to zero to avoid garbage data
  for (int i = 0; i < BUFFER_SIZE; i++) {
    stdinPipe.buffer[i] = 0;
    stderrPipe.buffer[i] = 0;
  }
  
  // Reset semaphores to ensure proper values
  destroySemaphore(stdinPipe.mutex);
  destroySemaphore(stdinPipe.written);
  destroySemaphore(stdinPipe.empty);
  
  stdinPipe.mutex = initSem(1);        // Only one process can access the buffer at a time
  stdinPipe.written = initSem(0);      // Initially, no data is available to read
  stdinPipe.empty = initSem(BUFFER_SIZE); // The entire buffer is empty initially
}

int64_t createPipe() {
  Pipe* p = globalMalloc(sizeof(Pipe));
  p->mutex = initSem(1);
  p->written = initSem(0);
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
  waitSemaphore(stdinPipe.empty);  // Wait until there's space in the buffer
  waitSemaphore(stdinPipe.mutex);  // Get exclusive access to the buffer
  stdinPipe.buffer[stdinPipe.producingIndex] = c;
  stdinPipe.producingIndex = (stdinPipe.producingIndex + 1) % BUFFER_SIZE;
  postSemaphore(stdinPipe.mutex);  // Release exclusive access
  postSemaphore(stdinPipe.written);  // Signal that there's data to read
}

int64_t readStdin(char* buf, int32_t len) {
  if (len <= 0) return 0;
  
  // Non-blocking check if data is available
  if (stdinPipe.consumingIndex == stdinPipe.producingIndex) {
    return 0; // No data available
  }
  
  // For stdin, read exactly one character at a time
  waitSemaphore(stdinPipe.written);
  waitSemaphore(stdinPipe.mutex);
  
  // Copy the character from the buffer
  *buf = stdinPipe.buffer[stdinPipe.consumingIndex];
  
  // Clear the character from the buffer after reading it
  stdinPipe.buffer[stdinPipe.consumingIndex] = 0;
  
  // Update the consuming index
  stdinPipe.consumingIndex = (stdinPipe.consumingIndex + 1) % BUFFER_SIZE;
  
  // Release the mutex and signal that we've freed a space
  postSemaphore(stdinPipe.mutex);
  postSemaphore(stdinPipe.empty);
  
  // Return the number of characters read (always 1)
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
  // setAtArrayIdx will do the free of the pipe itself when it overrides this position.
  return true;
}