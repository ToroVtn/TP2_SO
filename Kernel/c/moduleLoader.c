#include <lib.h>
#include <moduleLoader.h>
#include <stdint.h>

static void* loadModule(uint8_t** module, void* targetModuleAddress);
static uint32_t readUint32(uint8_t** address);

// loadModules(&endOfKernelBinary, moduleAddresses);
void* loadModules(void* payloadStart, void** targetModuleAddress) {
  // currentModule -> endOfKernelBinary
  uint8_t* currentModule = (uint8_t*)payloadStart;

  // moduleCount = endOfKernelBinary = 2
  // currentModule = &endOfKernelBinary + 32bits -> 1st module's size
  uint32_t moduleCount = readUint32(&currentModule);

  void* endOfModules;
  for (int i = 0; i < moduleCount; i++) endOfModules = loadModule(&currentModule, targetModuleAddress[i]);

  return endOfModules;
}

static void* loadModule(uint8_t** module, void* targetModuleAddress) {
  // Values for first loadModule call:
  // moduleSize = *(&endOfKernelBinary + 32)
  // module -> currentModule = &endOfKernelBinary + 64bits
  uint32_t moduleSize = readUint32(module);

  // This will take the current module, which was placed right after the
  // module count by the module packer (aka at &endOfKernelBinary + 64),
  // and copy it into the address we specified (0x400000 and 0x500000).
  memcpy(targetModuleAddress, *module, moduleSize);

  // *module = currentModule = &endOfKernelBinary + 64bits + moduleSize
  *module += moduleSize;

  return targetModuleAddress + moduleSize;
}

static uint32_t readUint32(uint8_t** address) {
  // address -> currentModule -> endOfKernelBinary
  // *address = currentModule -> endOfKernelBinary
  // result = **address = endOfKernelBinary = 2 --> At Toolchain/ModulePacker/main.c, the
  // number of modules is placed at the end of the kernel binaryendOfKernelBinary. That's why
  // we can retrieve that value here now.
  uint32_t result = *(uint32_t*)(*address);
  // *address = currentModule = &endOfKernelBinary + 32 bits -> 1st module's size
  *address += sizeof(uint32_t);
  return result;
}