section .text

global disableInterruptions
global enableInterruptions
global haltTillNextInterruption
	
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

extern irqDispatcher
extern readKeyCode
extern saveRegisters
extern exceptionDispatcher
extern getStackBase

%macro pushState 0
	push r15
  lea r15, [rsp + 8] ; me guardo el stack pointer al entrar a la interrupción
	push r14
	push r13
	push r12
	push r11
	push r10
	push r9
	push r8
	push rsi
	push rdi
  push qword [r15 + 24] ; el rsp del contexto anterior (que se encuentra en el interrupt stack)
	push rbp
	push rdx
	push rcx
	push rbx
	push rax
  push qword [r15] ; el rip
%endmacro

%macro popState 0
  pop r15 ; será sobreescrito
	pop rax
	pop rbx
	pop rcx
	pop rdx
	pop rbp
  pop r15 ; será sobreescrito
	pop rdi
	pop rsi
	pop r8
	pop r9
	pop r10
	pop r11
	pop r12
	pop r13
	pop r14
	pop r15
%endmacro

%macro irqHandler 1
  push rax
  mov rdi, %1
  call irqDispatcher
  mov al, 0x20
  out 0x20, al
  pop rax
  iretq
%endmacro

irq00Handler:
  irqHandler 0
irq01Handler:
  push rax
  call readKeyCode
  cmp al, 0x3b ; f1 para sacar captura de los registros
  pop rax
  jne .skip
  pushState
  push qword normalRegistersCode
  call saveRegisters
  pop rax
  popState
.skip:
  irqHandler 1
irq02Handler:
  irqHandler 2
irq03Handler:
  irqHandler 3
irq04Handler:
  irqHandler 4
irq05Handler:
  irqHandler 5
irq06Handler:
  irqHandler 6
irq07Handler:
  irqHandler 7

%macro exceptionHandler 1
  pushState
  push qword exceptionRegistersCode ; código para guardarlos en el arreglo de registros para excepciones, no el de hotkey
  call saveRegisters
  pop rax
  popState
  push rax
  mov rdi, %1
  call exceptionDispatcher
  mov al, 0x20
  out 0x20, al
  pop rax
  call getStackBase
  mov [rsp+24], rax
  mov rax, userland
  mov [rsp], rax
  iretq
%endmacro

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
