#include <stdint.h>
#include <syscalls.h>

uint32_t GetUint();
uint32_t GetUniform(uint32_t max);
uint8_t memcheck(void *start, uint8_t value, uint32_t size);
void* setMem(void* destiation, int32_t c, uint64_t length);
int64_t satoi(char *str);
void bussy_wait(uint64_t n);
void endless_loop(int32_t argc, char *argv[]);
void endless_loop_print(int32_t argc, char *argv[]);