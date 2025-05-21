section .text

global _start
_start:
	call main
	call halt


main:
	sub rsp, 8 ; Stack frame creation
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
	add rcx, 0 + 72
	mov rbp, rcx

	mov rax, 0100000000010100000000000000000000000000000000000000000000000000b
	push rax

	pop r8 ; assigning value to variable
	mov rdi, rbp
	add rdi, 8 * 0
	mov [rdi], r8

	mov r8, [rbp + 8 * 0]
	push r8

	call func_0_1
	add rsp, 8
	push rax

	call out
	add rsp, 8

	mov rax, 0000000000000000000000000000000000000000000000000000000000000000b
	push rax

	pop rax ; return
	pop rbp
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rdx
	pop rcx
	pop rbx
	add rsp, 8
	ret



func_0_1:
	sub rsp, 8 ; Stack frame creation
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
	add rcx, 0 + 72
	mov rbp, rcx
	mov rbx, [rbp + 16]
	mov [rbp - 0], rbx

	mov r8, [rbp + 8 * 0]
	push r8

	mov rax, 0011111111110000000000000000000000000000000000000000000000000000b
	push rax

	movsd xmm0, [rsp] ; moving tmp variables from stack to xmm registers
	add rsp, 8
	movsd xmm1, [rsp]
	add rsp, 8
	cmpsd xmm1, xmm0, 0 ; equality check
	movq [rsp], xmm1
	sub rsp, 8
	pop r8
	shr r8, 31
	push r8

	pop rdx ; condition
	cmp rdx, 0
	jne .label0
	mov rax, 0011111111110000000000000000000000000000000000000000000000000000b
	push rax

	pop rax ; return
	pop rbp
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rdx
	pop rcx
	pop rbx
	add rsp, 8
	ret


.label0:
	mov r8, [rbp + 8 * 0]
	push r8

	mov r8, [rbp + 8 * 0]
	push r8

	mov rax, 0011111111110000000000000000000000000000000000000000000000000000b
	push rax

	movsd xmm0, [rsp] ; moving tmp variables from stack to xmm registers
	add rsp, 8
	movsd xmm1, [rsp]
	add rsp, 8
	subsd xmm1, xmm0 ; substraction
	sub rsp, 8
	movq [rsp], xmm1

	call func_0_1
	add rsp, 8
	push rax

	movsd xmm0, [rsp] ; moving tmp variables from stack to xmm registers
	add rsp, 8
	
	movsd xmm1, [rsp]
	add rsp, 8
	mulsd xmm1, xmm0 ; multiplication
	sub rsp, 8
	movq [rsp], xmm1

	pop rax ; return
	pop rbp
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rdx
	pop rcx
	pop rbx
	add rsp, 8
	ret



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


halt:
	mov rax, 0x3c
	syscall

