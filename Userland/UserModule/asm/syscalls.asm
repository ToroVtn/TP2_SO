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
global sysCreateProcess
global sysCreateProcessWithPipeSwap
global sysExit
global sysWaitPid
global sysPCBList
global sysCreateSemaphore
global sysDestroySemaphore
global syswaitSem
global sysPostSem
global sysOpenSem
global sysMemcpy
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
global sysGetGlobalMemoryState
global sysGetProcessMemoryState



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
sysGetGlobalMemoryState:
  syscall 17
sysGetProcessMemoryState:
  syscall 18
sysCreateProcess:
  syscall 19
sysCreateProcessWithPipeSwap:
  syscall 20
sysExit:
  syscall 21
sysWaitPid:
  syscall 22
sysPCBList:
  syscall 23
sysSemInit:
  syscall 24
sysCreateSemaphore:
  syscall 25
sysDestroySemaphore:
  syscall 26
sysDestroySemaphoreByName:
  syscall 27
syswaitSem:
  syscall 28
sysPostSem:
  syscall 29
sysOpenSem:
  syscall 30
sysMemcpy:
  syscall 31
sysGetPid:
  syscall 32
sysKill:
  syscall 33
sysSleep:
  syscall 34
sysSetPriority:
  syscall 35
sysPipeInit:
  syscall 36
sysDestroyPipe:
  syscall 37
sysChangePipeRead:
  syscall 38
sysChangePipeWrite:
  syscall 39
sysFetchPipes:
  syscall 40
sysRead:
  syscall 41
sysWrite:
  syscall 42
sysBlockByUser:
  syscall 43
sysUnblock:
  syscall 44
sysYield:
  syscall 45