#include <stdint.h>
#include <lib.h>
#include <moduleLoader.h>

static void loadModule(uint8_t ** module, void * targetModuleAddress);
static uint32_t readUint32(uint8_t ** address);

// 	 loadModules(&endOfKernelBinary, moduleAddresses);
void loadModules(void * payloadStart, void ** targetModuleAddress)
{
	int i;
  // currentModule -> endOfKernelBinary
	uint8_t * currentModule = (uint8_t*)payloadStart;
  // moduleCount = endOfKernelBinary
	uint32_t moduleCount = readUint32(&currentModule);

	for (i = 0; i < moduleCount; i++)
		loadModule(&currentModule, targetModuleAddress[i]);
}

static void loadModule(uint8_t ** module, void * targetModuleAddress)
{
	uint32_t moduleSize = readUint32(module);
	memcpy(targetModuleAddress, *module, moduleSize);
	*module += moduleSize;
}

static uint32_t readUint32(uint8_t ** address)
{
  // address -> currentModule -> endOfKernelBinary
  // *address = currentModule -> endOfKernelBinary
  // result = **address = endOfKernelBinary = 2???
	uint32_t result = *(uint32_t*)(*address);
  // *address = currentModule -> endOfKernelBinary
	*address += sizeof(uint32_t);
  // *address = currentModule + 32 bits = &endOfKernelBinary + 32 bits
	return result;
}
