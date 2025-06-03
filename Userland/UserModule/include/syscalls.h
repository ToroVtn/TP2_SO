#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <colors.h>
#include <keyboard.h>
#include <sysinfo.h>
#include <processes.h>

extern void sysHalt();
extern int sysGetTicks();
extern void sysInfo(SystemInfo* si);
extern void sysSetLayout(int layoutIdx);
extern int sysSetFontSize(int fontSize);
extern void sysSetColor(FontColors c, uint32_t hexColor);
extern int sysRead(KeyStruct* buf, int len);
extern int sysWrite(int col, int row, char c);
extern int sysWriteCharXY(int x, int y, char c, int fontSize);
extern int sysWriteCharNext(char c);
extern int sysMoveCursor(int col, int row);
extern void sysPrintPixel(int x, int y, RGBColor color);
extern void sysFillRectangle(int x, int y, int width, int height, RGBColor color);
extern void sysPlaySound(uint32_t nFrequence, int ms);
extern void sysGetCurrentTime(Time* currentTime);
extern void sysGetRegisters(Register* registers);
extern void* sysMalloc(uint64_t size);
extern void sysFree(void* ptr);
extern uint32_t sysCreateProcess(int argc, char* argv[], void* procRip);
extern void sysExit(int exitCode);
extern int sysWaitPid(uint32_t pid);
extern PCB* sysPCBList(int* len);
extern int sysCreateSemaphore(char* name, int value);
extern int sysDestroySemaphore(char* name);
extern int sysWaitSem(int semId);
extern int sysPostSem(int semId);
extern int sysOpenSem(char* name, int value);

#endif