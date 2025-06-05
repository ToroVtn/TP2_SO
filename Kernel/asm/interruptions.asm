%include "/root/Kernel/asm/include/generalMacros.asm"


global disableInterruptions
global enableInterruptions
global haltTillNextInterruption
	
global picMask

global TTIrqHandler
global KBIrqHandler

global exception00Handler
global exception01Handler

global switcher
global switcherInterruption

extern irqDispatcher
extern readKeyCode
extern saveRegisters
extern exceptionDispatcher
extern getStackBase
extern schedule


section .text

TTIrqHandler:
  ;irqHandler 0
  pushAllRegs
  mov rdi, rsp
  call schedule
  mov rsp, rax
  mov rdi, 0
  call irqDispatcher
  popAllRegs
  eoi
  iretq

switcherInterruption:
  int 0x22
  ret

switcher:
  pushAllRegs
  mov rdi, rsp
  call schedule
  mov rsp, rax
  popAllRegs
  mov al, 0x20
  out 0x20, al
  iretq

KBIrqHandler:
.captureRegisters:
  push rax
  call readKeyCode
  cmp al, 0x3b ; f1 para sacar captura de los registros
  jne .nextProc
  pop rax
  pushState
  push qword normalRegistersCode
  call saveRegisters
  add rsp, 8 
  popState
  mov al, 0x20
  out 0x20, al
  iretq
.nextProc: 
  cmp al, 0x3c ; f2
  pop rax
  jne .regularKP ;regular key pressed
.f2Press:
  int 0x22
  mov al, 0x20
  out 0x20, al
  iretq
.regularKP:
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