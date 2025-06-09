#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <colors.h>
#include <keyboard.h>
#include <sysinfo.h>
#include <processes.h>
#include <pipes.h>

extern void sysHalt();
extern int sysGetTicks();
extern void sysInfo(SystemInfo* si);
extern void sysSetLayout(int layoutIdx);
extern int sysSetFontSize(int fontSize);
extern void sysSetColor(FontColors c, uint32_t hexColor);
extern int sysRead(int32_t pipe, char* buf, int len);
extern int sysWrite(int32_t pipe, const char* buf, int32_t len);
extern int sysWriteCharXY(int x, int y, char c, int fontSize);
extern int sysWriteCharNext(char c);
extern int sysMoveCursor(int col, int row);
extern void sysPrintPixel(int x, int y, RGBColor color);
extern void sysFillRectangle(int x, int y, int width, int height, RGBColor color);
extern void sysPlaySound(uint32_t nFrequence, int ms);
extern void sysGetCurrentTime(Time* currentTime);
extern void sysGetRegisters(Register* registers);
extern void sysGetModKeys(ModifierKeys* modKeys);
extern void* sysMalloc(uint64_t size);
extern void sysFree(void* ptr);
extern uint32_t sysCreateProcess(int argc, char* argv[], void* procRip);
extern void sysExit(int exitCode);
extern int sysWaitPid(uint32_t pid);
extern PCB* sysPCBList(int* len);
extern int sysCreateSemaphore(char* name, int value);
extern int sysDestroySemaphore(int name);
extern int sysWaitSem(int semId);
extern int sysPostSem(int semId);
extern int sysOpenSem(char* name, int value);
extern uint32_t sysGetPid();
extern bool sysKill(uint32_t pid);
extern void sysSleep(uint64_t ms);
extern void sysSetPriority(uint32_t pid, uint32_t newPriority);
extern bool sysBlockByUser(uint32_t pid);
extern bool sysUnblock(uint32_t pid);
extern bool sysDestroyPipe(int32_t pipeId);
extern int32_t sysPipeInit();
extern Pipe sysFetchPipes();
extern int32_t sysCreateProcessWithPipeSwap(int argc, const char* argv[], void* procRip, Pipe pipes);
extern void* sysMemcpy(void* destination, const void* source, uint64_t length);
extern char* sysGetGlobalMemoryState();
extern char* sysGetProcessMemoryState(uint32_t pid);
extern bool sysDestroySemaphoreByName(char* name);
extern int32_t sysSemInit(uint32_t initialValue);
extern void sysYield();
#endif