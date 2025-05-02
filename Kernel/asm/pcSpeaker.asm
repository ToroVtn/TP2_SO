section .text
global playSound
global noSound

; fuente: (https://wiki.osdev.org/PC_Speaker)

playSound:
    ; void playSound(uint32_t nFrequence)

    push rbp
    mov rbp, rsp
    push rdx

    xor rdx, rdx           ; Limpiar RDX (para la división)
    mov rax, baseFrequence ; Cargar la frecuencia base
    div rdi                ; Dividir por la frecuencia deseada
    mov rdx, rax           ; Guardo la división para usarla después

    mov al, 0b10110110     ; Configuración del temporizador PIT (https://wiki.osdev.org/Programmable_Interval_Timer)
    out 0x43, al
    mov al, dl             ; Parte baja del resultado
    out 0x42, al
    mov al, dh             ; Parte alta del resultado
    out 0x42, al

    in al, 0x61            ; Leer el estado actual del puerto 0x61
    or al, 3               ; Establecer los bits 0 y 1 para habilitar el altavoz (los 2 bits más bajos)
    out 0x61, al           ; Escribir de vuelta al puerto 0x61

    pop rdx
    mov rsp, rbp
    pop rbp
    ret

noSound:
    ; void noSound();

    push rbp
    mov rbp, rsp

    in al, 0x61 ; Leer el estado actual del puerto 0x61

    and al, 0xFC ; Limpiar los dos bits más bajos
    out 0x61, al ; Escribir de vuelta al puerto 0x61 para silenciar el altavoz

    mov rsp, rbp
    pop rbp
    ret

section .data
baseFrequence equ 1193180