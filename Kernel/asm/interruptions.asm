%include "/root/Kernel/asm/include/generalMacros.asm"


global disableInterruptions
global enableInterruptions
global haltTillNextInterruption
	
global picMask

global timerTickIrqHandler
global keyboardIrqHandler

global exception00Handler
global exception01Handler

global asdf
global asdfInterruption

extern irqDispatcher
extern readKeyCode
extern saveRegisters
extern exceptionDispatcher
extern getStackBase
extern schedule


section .text

timerTickIrqHandler:
  irqHandler 0

asdfInterruption:
  int 0x22
  ret

asdf:
  pushGpr
  mov rdi, rsp
  call schedule
  mov rsp, rax
  popGpr
  mov al, 0x20
  out 0x20, al
  iretq

keyboardIrqHandler:
.captureRegisters:
  push rax
  call readKeyCode
  cmp al, 0x3b ; f1 para sacar captura de los registros
  jne .nextProcess
  pop rax
  pushState
  push qword normalRegistersCode
  call saveRegisters
  add rsp, 8 ; remove the pushed normalRegistersCode from stack
  popState
  mov al, 0x20
  out 0x20, al
  iretq
.nextProcess: ; Just for testing
  cmp al, 0x3c ; f2
  pop rax
  jne .regularKeyPress
.f2Press:
  int 0x22
  mov al, 0x20
  out 0x20, al
  iretq
.regularKeyPress:
  irqHandler 1

exception00Handler:
  exceptionHandler 0
exception01Handler:
  exceptionHandler 1

disableInterruptions:
	cli
	ret

enableInterruptions:
	sti
	ret

haltTillNextInterruption:
  sti
  hlt
  ret

picMask:
  mov ax, di
  out	0x21, al
  shr ax, 8
  out	0xA1, al
  retn

section .rodata
  normalRegistersCode equ 1
  exceptionRegistersCode equ 0
  userland equ 0x400000