GLOBAL setBinaryClockFormat
GLOBAL getCurrentSeconds
GLOBAL getCurrentMinutes
GLOBAL getCurrentHours 

section .text

; fuente: (https://stanislavs.org/helppc/cmos_ram.html)

setBinaryClockFormat:
	push rbp
	mov rbp, rsp

	mov rax, 0x0B ; consigo el RTC Status Register B
	out 70h, al
	mov rax, 0b00000110 ; le cambio la configuración para devuelva la hora en formato binario en lugar de BCD
	out 71h, al

	mov rsp, rbp
	pop rbp
	ret

getCurrentSeconds:
	push rbp
	mov rbp, rsp

	mov rax, 0x00 ; configuración para obtener los segundos
	out 70h, al
	in al, 71h

	mov rsp, rbp
	pop rbp
	ret

getCurrentMinutes:
	push rbp
	mov rbp, rsp

	mov rax, 0x02 ; configuración para obtener los minutos
	out 70h, al
	in al, 71h

	mov rsp, rbp
	pop rbp
	ret

getCurrentHours:
	push rbp
	mov rbp, rsp
	push rdx
	mov rdx, 24

	mov rax, 0x04 ; configuración para obtener las horas
	out 70h, al
	in al, 71h
	sub al, 3 ; pasaje del horario internacional al argentino
	cmp al, 0
	jge .fin
	add al, 24
.fin:
	pop rdx
	mov rsp, rbp
	pop rbp
	ret