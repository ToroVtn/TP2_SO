section .text

extern getSyscallsArray



; -------------------------     FUNCTION     ----------------------------
; Arguments
;  r9: syscall index
;  rdi: first syscall arg   (optional)
;  rsi: second syscall arg  (optional)
;  rdx: third syscall arg   (optional)
;  rcx: fourth syscall arg   (optional)
;  r8: fifth syscall arg   (optional)
; Return:
;  rax (optional)
; -----------------------------------------------------------------------
syscallDispatcher:
  call getSyscallsArray
  lea rax, [rax + r9*8]
  call [rax]
  iretq

section .text

global sysHalt
global sysGetTicks
global sysInfo
global sysSetLayout
global sysSetFontSize
global sysSetColor
global sysRead
global sysWrite
global sysWriteCharXY
global sysWriteCharNext
global sysMoveCursor
global sysPrintPixel
global sysFillRectangle
global sysPlaySound
global sysGetCurrentTime
global sysGetRegisters
global sysMalloc
global sysCreateProcess,
global sysExit,

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
sysRead:
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
sysCreateProcess:
  syscall 16
sysExit:
  syscall 17
syscallDispatcher:
  syscall 18