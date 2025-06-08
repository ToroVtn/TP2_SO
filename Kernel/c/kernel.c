#include <clock.h>
#include <interruptions.h>
#include <lib.h>
#include <memory.h>
#include <moduleLoader.h>
#include <stdint.h>
#include <videoDriver.h>
#include <scheduler.h>
#include <semaphores.h>

// extern uint8_t kernelText;
// extern uint8_t kernelRodata;
// extern uint8_t kernelData;
extern uint8_t kernelBss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;

static const uint64_t PageSize = 0x1000;

typedef int (*EntryPoint)();

EntryPoint const userModule = (EntryPoint)0x400000;
static EntryPoint const sampleDataModule = (EntryPoint)0x500000;

extern void initUserModule();

void clearBSS(void* bssAddress, uint64_t bssSize) {
  memset(bssAddress, 0, bssSize);
}

void* getStackBase() {
  //Calculate the stack base address by taking the end of the kernel binary
  return (void*)((uint64_t)&endOfKernel + PageSize * 8 - sizeof(uint64_t));
}

void* initializeKernelBinary() {
  void* moduleAddresses[] = {userModule, sampleDataModule};

  void* endOfModules = loadModules(&endOfKernelBinary, moduleAddresses);

  clearBSS(&kernelBss, &endOfKernel - &kernelBss);

  memoryInit(endOfModules);

  setBinaryClockFormat();

  return getStackBase();
}

int main() {
  loadIdt();
  setFontGridValues();
  createPCBList();
  initSemArray();

  // userModule();
  initUserModule();


  return 0;
}