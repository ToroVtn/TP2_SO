#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <colors.h>
#include <keyboard.h>
#include <sysinfo.h>

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
void sysPrintPixel(int x, int y, RGBColor color);
void sysFillRectangle(int x, int y, int width, int height, RGBColor color);
extern void sysPlaySound(uint32_t nFrequence, int ms);
extern void sysGetCurrentTime(Time *currentTime);
extern void sysGetRegisters(Register * registers);

#endif
