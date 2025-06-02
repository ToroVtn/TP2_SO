%include "/root/Kernel/asm/include/generalMacros.asm"


global initializeProcessStack
global idleProc
global exit
global startUserModule

extern stackAlloc
extern exitCurrentProcess
extern createUserModuleProcess
extern asdfInterruption


section .text

; -------------------------     FUNCTION     ----------------------------
; Description: Creates new process
; Arguments
;  rdi: argc
;  rsi: argv
;  rdx: pointer to process function
;  rcx: pointer to begining of stack
; Return
;  rax: current stack pointer for created process
; -----------------------------------------------------------------------
initializeProcessStack:
  ; This is just so gdb detects this function for the call stack.
  push rbp
  mov rbp, rsp

  ; I need to store the original rsp in a register because I'll loose access to the current
  ; stack when I make the swap for the new process' stack.
  push r11
  mov r11, rsp

  ; Move the stack pointer to the allocated memory (this will be the new process' stack)
  mov rsp, rcx
  mov rbp, rcx
  push 0      ; ss
  push rcx    ; original rsp (before the pushes)
  push 0x202  ; rflags
  push 0x8    ; cs
  push rdx    ; rip

  ; Note that rdi and rsi still have the correct values of argc and argv
  initializeGpr

  mov rax, rsp ; Set final rsp value (after pushes) as return value

  ; Return to original stack
  mov rsp, r11
  pop r11
  pop rbp
  ret

; -------------------------     FUNCTION     ----------------------------
; Description: A process created at kernel initialization and which is always ready
; Arguments: None
; -----------------------------------------------------------------------
idleProc:
  hlt
  jmp idleProc

; -------------------------     FUNCTION     ----------------------------
; Description: Exit from process. Process stack and pcb will get cleared.
;              Then call timer tick interruption.
; Arguments
;  rdi: exit code (not currently used)
; Return: doesn't return
; -----------------------------------------------------------------------
exit:
  call exitCurrentProcess
  int 0x22

; -------------------------     FUNCTION     ----------------------------
; Description: Create usermodule process. Not trivial as I need to circunvent
; the timer tick interruption (keyboard one for now tho (f2)) to avoid 
; overriding rsp.
; Arguments: None
; Return: doesn't return
; -----------------------------------------------------------------------
startUserModule:
  call createUserModuleProcess
  mov rsp, rax
  popGpr
  eoi
  iretq