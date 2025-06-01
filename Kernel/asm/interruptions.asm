%include "asm/generalMacros.asm"

section .text

global disableInterruptions
global enableInterruptions
global haltTillNextInterruption
global switcher
global switcherInterruption
	
global picMask

global irq00Handler
global irq01Handler
global irq02Handler
global irq03Handler
global irq04Handler
global irq05Handler
global irq06Handler
global irq07Handler
global exception00Handler
global exception01Handler

global timerTickIrqHandler
global KBIrqHandler

extern irqDispatcher
extern readKeyCode
extern saveRegisters
extern exceptionDispatcher
extern getStackBase
extern schedule

switcherInterruption:
  int 0x22
  ret

switcher:
  pushAllRegs
  mov rdi, rsp
  call schedule
  mov rsp, rax
  popAllRegs
  eoi
  iretq

KBIrqHandler:
  push rax
  call readKeyCode
  cmp al, 0x3b
  jne .next
  add rsp, 8
  pushState
  push qword normalRegistersCode
  call saveRegisters
  pop rax
  popState
  eoi
  iretq

.next:
  cmp al, 0x3c 
  pop rax
  jne .regularKP
.f2:
  int 0x22 ; switcherInterruption  
    eoi
    iretq
.regularKP:
  irqHandler 1

;irq00Handler:
;  irqHandler 0
;irq01Handler:
;  push rax
;  call readKeyCode
;  cmp al, 0x3b ; f1 para sacar captura de los registros
;  pop rax
;  jne .skip
;  pushState
;  push qword normalRegistersCode
;  call saveRegisters
;  pop rax
;  popState
;.skip:
;  irqHandler 1
;irq02Handler:
;  irqHandler 2
;irq03Handler:
;  irqHandler 3
;irq04Handler:
;  irqHandler 4
;irq05Handler:
;  irqHandler 5
;irq06Handler:
;  irqHandler 6
;irq07Handler:
;  irqHandler 7

timerTickIrqHandler:
  irqHandler 0

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
	; push rbp
  ; mov rbp, rsp

  mov ax, di
  out	0x21, al
  shr ax, 8
  out	0xA1, al

  ; mov rsp, rbp
  ; pop rbp
  retn  ; return near: returns to address in same code segment.
        ; retf -> return far: can change code segment. Not used in
        ; modern systems as they use a single code segment.
        ; ret: compiler decides which of the above should be used. Basically same as retn


section .rodata
  normalRegistersCode equ 1
  exceptionRegistersCode equ 0
  userland equ 0x400000
