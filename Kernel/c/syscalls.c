#include <clock.h>
#include <interruptions.h>
#include <keyboard.h>
#include <memory.h>
#include <pcSpeaker.h>
#include <registers.h>
#include <scheduler.h>
#include <syscalls.h>
#include <sysinfo.h>
#include <timer.h>
#include <videoDriver.h>

/*
 * There should be stdin, stdout and stderr global variables and read/write syscalls that get/set them.
 */

static SyscallFunction syscalls[] = {
    (SyscallFunction)haltTillNextInterruption,
    (SyscallFunction)getTicks,
    (SyscallFunction)getSysInfo,
    (SyscallFunction)setLayout,
    (SyscallFunction)setFontSize,
    (SyscallFunction)setColor,
    (SyscallFunction)readKbBuffer,
    (SyscallFunction)printCharXY,
    (SyscallFunction)printNextChar,
    (SyscallFunction)moveCursor,
    (SyscallFunction)printPixel,
    (SyscallFunction)fillRectangle,
    (SyscallFunction)playSoundForCertainMs,
    (SyscallFunction)getCurrentTime,
    (SyscallFunction)getRegisters,
    (SyscallFunction)malloc,
    (SyscallFunction)free,
    (SyscallFunction)initUserProc,
    (SyscallFunction)exit,
    (SyscallFunction)waitPid,
    (SyscallFunction)fetchPCBList,
};

SyscallFunction* getSyscallsArray() {
  return syscalls;
}