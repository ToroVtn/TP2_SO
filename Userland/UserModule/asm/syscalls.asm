section .text

global sysHalt
global sysGetTicks
global sysInfo
global sysSetLayout
global sysSetFontSize
global sysSetColor
global sysGetModKeys
global sysReadKbBuffer
global sysWriteCharXY
global sysWriteCharNext
global sysMoveCursor
global sysPrintPixel
global sysFillRectangle
global sysPlaySound
global sysGetCurrentTime
global sysGetRegisters
global sysMalloc
global sysFree
global sysCreateProcess,
global sysExit,
global sysWaitPid,
global sysPCBList
global sysCreateSemaphore
global sysDestroySemaphore
global sysWaitSem
global sysPostSem
global sysOpenSem
global sysGetPid
global sysKill
global sysSleep
global sysSetPriority
global sysPipeInit
global sysDestroyPipe
global sysChangePipeRead
global sysChangePipeWrite
global sysFetchPipes
global sysRead
global sysWrite
global sysBlockByUser
global sysUnblock
global sysYield



%macro syscall 1
  mov r9, %1
  int 0x80
  ret
%endmacro

sysHalt:
  syscall 0
sysGetTicks:
  syscall 1
sysInfo:
  syscall 2
sysSetLayout:
  syscall 3
sysSetFontSize:
  syscall 4
sysSetColor:
  syscall 5
sysGetModKeys:
  syscall 6
;sysReadKbBuffer:
;  syscall 7
sysWriteCharXY:
  syscall 7
sysWriteCharNext:
  syscall 8
sysMoveCursor:
  syscall 9
sysPrintPixel:
  syscall 10
sysFillRectangle:
  syscall 11
sysPlaySound:
  syscall 12
sysGetCurrentTime:
  syscall 13
sysGetRegisters:
  syscall 14
sysMalloc:
  syscall 15
sysFree:
  syscall 16
sysCreateProcess:
  syscall 17
sysExit:
  syscall 18
sysWaitPid:
  syscall 19
sysPCBList:
  syscall 20
sysCreateSemaphore:
  syscall 21
sysDestroySemaphore:
  syscall 22
sysWaitSem:
  syscall 23
sysPostSem:
  syscall 24
sysOpenSem:
  syscall 25
sysGetPid:
  syscall 26
sysKill:
  syscall 27
sysSleep:
  syscall 28
sysSetPriority:
  syscall 29
sysPipeInit:
  syscall 30
sysDestroyPipe:
  syscall 31
sysChangePipeRead:
  syscall 32
sysChangePipeWrite:
  syscall 33
sysFetchPipes:
  syscall 34
sysRead:
  syscall 35
sysWrite:
  syscall 36
sysBlockByUser:
  syscall 37
sysUnblock:
  syscall 38
sysYield:
  syscall 39
