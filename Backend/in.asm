section .text
global _start
_start:
	call in
	push rax
	call out
	mov rax, 0x3c
	syscall

in:
	sub rsp, 64 ; Stack frame creation
	push rbx
	push rcx
	push rdx
	push rdi
	push rsi
	push r8
	push r9
	push r10
	push rbp
	mov rcx, rsp
	add rcx, 56 + 72
	mov rbp, rcx
	mov rdi, rbp 
	sub rdi, 56

	mov rax, 0
	mov rsi, rdi
	mov rdi, 0
	mov rdx, 64
	syscall
	mov rcx, rax
	mov rdi, rbp
	sub rdi, 56
	sub rcx, 2
	add rdi, rcx
	inc rcx

	xor r8, r8
	push r8
	movsd xmm0, [rsp]
	add rsp, 8
.ReadAsIntegerLoop:
cmp r8, rcx
je .ReadAsIntegerLoopEnd
xor rbx, rbx
mov bl, byte [rdi]
cmp rbx, 0x2E
je .InDivisorAppeared
sub rbx, 0x30
shl rbx, 3
mov rax, [FloatNumber + rbx]
push rax
movsd xmm1, [rsp]
add rsp, 8
jmp .PowerOfTen
.PowerOfTenRet:
addsd xmm0, xmm1
dec rdi
inc r8
jmp .ReadAsIntegerLoop
.ReadAsIntegerLoopEnd:
mov rax, 0x4024000000000000
push rax
movsd xmm1, [rsp]
add rsp, 8
.DivisionLoop:
cmp r9, 0
je .DivisionLoopEnd
divsd xmm0, xmm1
dec r9
jmp .DivisionLoop
.DivisionLoopEnd:
movq rax, xmm0
pop rbp
pop r10
pop r9
pop r8
pop rsi
pop rdi
pop rdx
pop rcx
pop rbx
add rsp, 64
ret
.InDivisorAppeared:
	xor r9, r9
	mov r9, r8
	dec rdi
	jmp .ReadAsIntegerLoop



.PowerOfTen:
	xor r10, r10
	push r10
	movsd xmm2, [rsp]
	add rsp, 8
	mov r10, r8
	mov rax, 0x4024000000000000
	push rax
	movsd xmm2, [rsp]
	add rsp, 8
.TakingPowerLoop:
	cmp r10, 0
	je .TakingPowerLoopEnd
	dec r10
	mulsd xmm1, xmm2
	jmp .TakingPowerLoop

.TakingPowerLoopEnd:
	jmp .PowerOfTenRet


out:
	pop rax
	sub rsp, 80
	push rbp
	mov rbp, rsp
	add rbp, 88
	push rbx
	push rcx
	push rdx
	push rdi
	push rsi
	push r9
	mov r9, rax
	mov rdi, rbp
	sub rdi, 1
	mov qword rax, [rbp]

	mov rbx, 01h
	mov rdx, rax
	mov rcx, 64
	mov byte [rdi], 0x0A
	dec rdi
.PrintingLoop:
	mov rdx, rax
	cmp rcx, 0x00
	je .PrintingLoopEnd
	and rdx, rbx
	add rdx, 0x30
	mov byte [rdi], dl
	dec rdi
	shr rax, 1
	dec rcx
	jmp .PrintingLoop
.PrintingLoopEnd:
	mov rax, 0x01
	mov rsi, rdi
	mov rdi, 0x01
	mov rdx, 66
	syscall
	sub rbp, 8
	mov [rbp], r9
	pop r9
	pop rsi
	pop rdi
	pop rdx
	pop rsi
	pop rbx
	pop rbp
	add rsp, 72
	ret

section .data
	FloatNumber dq 0, 0x3FF0000000000000, 0x4000000000000000, 0x4008000000000000, 0x4010000000000000, 0x4014000000000000, 0x4018000000000000, 0x401C000000000000, 0x4020000000000000, 0x4022000000000000