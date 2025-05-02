#include <interruptions.h>
#include <keyboard.h>
#include <syscalls.h>
#include <sysinfo.h>
#include <timer.h>
#include <videoDriver.h>
#include <pcSpeaker.h>
#include <clock.h>
#include <registers.h>

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
  (SyscallFunction)getRegisters
};

SyscallFunction* getSyscallsArray() {
  return syscalls;
}
