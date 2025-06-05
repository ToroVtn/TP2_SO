%macro pushState 0
	push r15
  lea r15, [rsp + 8] 
	push r14
	push r13
	push r12
	push r11
	push r10
	push r9
	push r8
	push rsi
	push rdi
  push qword [r15 + 24] 
	push rbp
	push rdx
	push rcx
	push rbx
	push rax
  push qword [r15] 
%endmacro

%macro popState 0
  pop r15 
	pop rax
	pop rbx
	pop rcx
	pop rdx
	pop rbp
  pop r15 
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
  push qword exceptionRegistersCode 
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

%macro pushAllRegs 0
  push rax
  push rbx
  push rcx
  push rdx
  push rsi
  push rdi
  push rbp
  push r8
  push r9
  push r10
  push r11
  push r12
  push r13
  push r14
  push r15
%endmacro

%macro popAllRegs 0
  pop r15
  pop r14
  pop r13
  pop r12
  pop r11
  pop r10
  pop r9
  pop r8
  pop rbp
  pop rdi
  pop rsi
  pop rdx
  pop rcx
  pop rbx
  pop rax
%endmacro

%macro initRegs 0
  push 0
  push 0
  push 0
  push 0
  push rsi  ; argv
  push rdi  ; argc
  push rbp
  push 0
  push 0
  push 0
  push 0
  push 0
  push 0
  push 0
  push 0
%endmacro