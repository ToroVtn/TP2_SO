
%include "asm/generalMacros.asm"

global createStack
global idleProcess
global exit
global userModInit

extern allocateStack
extern exitProcess
extern userModProcessInit

section .text

; -------------------------     ROUTINE     ----------------------------
; Parameters: 
; rdi: argc (number of arguments)
; rsi: argv (array of argument strings)
; rdx: process function pointer
; rcx: pointer to stack base
; Returns: pointer to the new process stack
createStack:
    ; This is just so gdb detects this function for the call stack.
    push rbp       ; stackframe
    mov rbp, rsp

    ; Store original rsp as we'll lose access to current stack during swap
    push r11
    mov r11, rsp

    ; Preserve registers that may be modified by stackAlloc
    mov rsp, rcx
    mov rbp, rcx
    

    ; Set up stack frame for new process   
    push 0      ; ss
    push rcx    ; original rsp
    push 0x202  ; rflags
    push 0x8    ; cs
    push rdx    ; rip (process function pointer)

    ; Initialize general purpose registers
    initRegs

    ; Save final stack pointer as return value
    mov rax, rsp

    ; Restore original stack
    mov rsp, r11
    pop r11
    pop rbp
    ret
; -----------------------------------------------------------------------

; -------------------------     ROUTINE     ----------------------------
; Parameters; none
; Returns: None
userModInit:
    call userModProcessInit
    mov rsp, rax
    popAllRegs
    eoi
    iretq

; -------------------------     ROUTINE     ----------------------------
; Desc: A process created at kernel initialization and which is always ready
; Parameters: None
; Return: None

idleProcess:
    hlt
    jmp idleProcess
; -----------------------------------------------------------------------

; -------------------------     ROUTINE     ----------------------------
; Desc: Exit from process. Process stack and pcb will get cleared.
;              Then call timer tick interruption.
; Parameters:
;  rdi: exit code (not currently used)
; Return: doesn't return
exit:
    call exitProcess
    int 0x22
; -----------------------------------------------------------------------

