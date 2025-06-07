%include "/root/Kernel/asm/include/generalMacros.asm"


global initStack
global idleProc
global initUserModule

extern allocateStack
extern exitProc
extern initUserModuleProc
extern switcherInterruption


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
initStack:
  
  push rbp
  mov rbp, rsp

  
  push r11
  mov r11, rsp

  mov rsp, rcx
  mov rbp, rcx
  push 0      
  push rcx    
  push 0x202  
  push 0x8    
  push rdx    

  initRegs

  mov rax, rsp 

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
; Description: Create usermodule process. Not trivial as I need to circunvent
; the timer tick interruption (keyboard one for now tho (f2)) to avoid 
; overriding rsp.
; Arguments: None
; Return: doesn't return
; -----------------------------------------------------------------------
initUserModule:
  call initUserModuleProc
  mov rsp, rax
  popAllRegs
  sti
  eoi
  iretq