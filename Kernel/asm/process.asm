
%include "asm/include/generalMacros.asm"

global createStack
global idleProcess
global exit

extern allocateStack
extern processExit

section .text

; -------------------------     ROUTINE     ----------------------------
; Parameters: 
; rdi: argc (number of arguments)
; rsi: argv (array of argument strings)
; rdx: process function pointer
; Returns: pointer to the new process stack
createStack:
    ; This is just so gdb detects this function for the call stack.
    push rbp       ; stackframe
    mov rbp, rsp

    ; Store original rsp as we'll lose access to current stack during swap
    push r11
    mov r11, rsp

    ; Preserve registers that may be modified by stackAlloc
    push rdi    ; argc
    push rsi    ; argv
    push rdx    ; process function pointer

    ; Allocate new stack
    mov rdi, [processStackSize]
    call allocateStack

    ; Restore preserved registers
    pop rdx     ; process function pointer
    pop rsi     ; argv
    pop rdi     ; argc

    ; Set up new process stack
    mov rsp, rax
    mov rbp, rax

    ; Set up stack frame for new process
    push 0      ; Align
    push 0      ; ss
    push rax    ; original rsp
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
; Description: A process created at kernel initialization and which is always ready
; Arguments: None
; Return: None

idleProcess:
    hlt
    jmp idleProcess
; -----------------------------------------------------------------------

; -------------------------     ROUTINE     ----------------------------
; Description: Exit from process. Process stack and pcb will get cleared.
;              Then call timer tick interruption.
; Arguments
;  rdi: exit code (not currently used)
; Return: doesn't return
processExit:
    call exitProcess
    int 0x20
; -----------------------------------------------------------------------

section .rodata
    processStackSize dq 0x1000 