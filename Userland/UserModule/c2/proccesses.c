#include <processes.h>
#include <shellUtils.h>
#include <syscalls.h>
#include <stdint.h>

void printPCBList(PCB* pcbList, int32_t len) {
  printf("%3s, %-14s, %-9s, %-5s, %10s, %10s, %8s\n", "PID", "Name", "State", "Fg/Bg", "rsp", "rbp", "Priority");
  for (int32_t i = 0; i < len; ++i) {
    PCB* pcb = pcbList + i;
    printf(
        "%3u, %-14s, %-9s, %-5s, %p, %p, %8d\n", pcb->pid, pcb->name, pcb->state, pcb->location, pcb->rsp, pcb->rbp,
        pcb->priority
    );
  }
  sysFree(pcbList);
}