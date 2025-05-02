#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <stdint.h>

extern uint64_t syscallDispatcher(uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e, int idx);

typedef uint64_t (*SyscallFunction)(uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e);

SyscallFunction* getSyscallsArray();

#endif
