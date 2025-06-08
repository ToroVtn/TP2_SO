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
#include <semaphores.h>
#include <pipes.h>
#include <lib.h>

/*
 * There should be stdin, STDOUT and stderr global variables and read/write syscalls that get/set them.
 */

static SyscallFunction syscalls[] = {
    (SyscallFunction)haltTillNextInterruption,
    (SyscallFunction)getTicks,
    (SyscallFunction)getSysInfo,
    (SyscallFunction)setLayout,
    (SyscallFunction)setFontSize,
    (SyscallFunction)setColor,
    (SyscallFunction)getModKeys,
    //(SyscallFunction)readKbBuffer,
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
    (SyscallFunction)initUserProcWithPipeSwap,
    (SyscallFunction)exitProc,
    (SyscallFunction)waitPid,
    (SyscallFunction)fetchPCBList,
    (SyscallFunction)createSem,
    (SyscallFunction)destroySemaphore,
    (SyscallFunction)waitSemaphore,
    (SyscallFunction)postSemaphore,
    (SyscallFunction)openSemaphore,
    (SyscallFunction)memcpy,
    (SyscallFunction)getpid,
    (SyscallFunction)kill,
    (SyscallFunction)sleep,
    (SyscallFunction)setPriority,
    (SyscallFunction)createPipe,
    (SyscallFunction)deletePipe,
    (SyscallFunction)changePipeRead,
    (SyscallFunction)changePipeWrite,
    (SyscallFunction)fetchPipes,
    (SyscallFunction)read,
    (SyscallFunction)write,
    (SyscallFunction)block,
    (SyscallFunction)unBlock,
    (SyscallFunction)yield

};

SyscallFunction* getSyscallsArray() {
  return syscalls;
}