section .data
registersQuantity equ 17

section .text

global saveRegisters

extern getRegisterValues
extern getExceptionRegisterValues

; paso los registros por stack
; el primer argumento es para ver en que array guardo los registros, 0 = exceptionRegisters, != 0 registers
saveRegisters:
    push rbp
    mov rbp, rsp
    push rax
    push rbx
    push rcx
    push rdx

    cmp qword [rbp + 16], 1 ; veo en que array lo guardaré
    je .noException

.exception:
    call getExceptionRegisterValues
    mov rbx, rax
    jmp .getFirstReg
.noException:
    call getRegisterValues
    mov rbx, rax      ; guardo la dirección donde almacenaré los registros    

.getFirstReg:
    lea rax, [rbp + 24]     ; guardo en rax la dirección del segundo argumento, es decir el rip pusheado anteriormente
    xor rcx, rcx            ; seteo el contador en 0

.loop:
    cmp cl, registersQuantity
    jge .end                ; saltar si cl es >= que 17, que es la cantidad de registros a almacenar

    mov rdx, [rax]          ; desreferencio rax para conseguir el argumento
    mov qword [rbx], rdx    ; lo guardo en registers para usarlo después
    add rax, 8              ; voy al siguiente argumento
    add rbx, 8              ; voy a la siguiente dirección en registers para guardar el sig argumento
    
    inc rcx                 ; incremento el contador
    jmp .loop

.end:

    pop rdx
    pop rcx
    pop rbx
    pop rax
    mov rsp, rbp
    pop rbp
    ret
