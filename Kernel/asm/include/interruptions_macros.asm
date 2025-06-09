
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

%macro eoi 0
  mov al, 0x20
  out 0x20, al
%endmacro

%macro irqHandler 1
  push rax
  mov rdi, %1
  call irqDispatcher
  eoi
  pop rax
  iretq
%endmacro

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