#include <stdint.h>

extern uint64_t registers[][16];

void saveRegisters(uint64_t* rsp, int code) {
    uint64_t* regs = registers[code];
    regs[0] = rsp[0];  // rax
    regs[1] = rsp[1];  // rbx
    regs[2] = rsp[2];  // rcx
    regs[3] = rsp[3];  // rdx
    regs[4] = rsp[4];  // rbp
    regs[5] = rsp[5];  // rdi
    regs[6] = rsp[6];  // rsi
    regs[7] = rsp[7];  // r8
    regs[8] = rsp[8];  // r9
    regs[9] = rsp[9];  // r10
    regs[10] = rsp[10]; // r11
    regs[11] = rsp[11]; // r12
    regs[12] = rsp[12]; // r13
    regs[13] = rsp[13]; // r14
    regs[14] = rsp[14]; // r15
    regs[15] = rsp[15]; // rip
} 