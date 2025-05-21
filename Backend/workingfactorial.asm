section .text

global _start
_start:
	call main
	call halt


main:

	sub rsp, 8 ; (lvn + an)
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
	add rcx, 72; (pushed registers + lnvn +an - 1)
	mov rbp, rcx

	;mov rbx, [rbp - 16] ; rbp - (1 + an) * 8, next is (rbp - (an) * 8)
	;mov [rbp], rbx ; rbp + argnum


	mov rax, 0x4014000000000000
	push rax

	pop r8 ; assigning value to variable
	mov rdi, rbp
	add rdi, 8 * 0
	mov [rdi], r8

	mov r8, [rbp + 8 * 0]
	push r8

	call func_0_1
	add rsp, 8 ; 8 * argnum
	push rax
    
	call out
	add rsp, 8
 ; function call	push ra
	mov rax, 0000000000000000000000000000000000000000000000000000000000000000b
	push rax

	pop rax

	pop rbp
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rdx
	pop rcx
	pop rbx

	add rsp, 8 ; (8 * (lvn + an))

	ret
	ret

func_0_1:

	sub rsp, 8 ; (lvn + an)
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
	add rcx, 72; (pushed registers + lnvn +an - 1)
	mov rbp, rcx

	mov rbx, [rbp + 16] ; rbp + (an - n + 1) * 8 (if has arguments)
	mov [rbp - 0], rbx ; rbp - argIndex

	mov r8, [rbp + 8 * 0]
	push r8

	mov rax, 0011111111110000000000000000000000000000000000000000000000000000b
	push rax

	movsd xmm0, [rsp] ; moving tmp variables from stack to xmm registers
	add rsp, 8
	movsd xmm1, [rsp]
	add rsp, 8
	cmpsd xmm1, xmm0, 0 ; equality check
	sub rsp, 8
	movq [rsp], xmm1
	pop r8
	shl r8, 63
	push r8

	pop rdx ; condition
	cmp rdx, 0
	je label0
	mov rax, 0011111111110000000000000000000000000000000000000000000000000000b
	push rax

	pop rax

	pop rbp
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rdx
	pop rcx
	pop rbx

	add rsp, 8 ; (8 * (lvn + an))

	ret

label0:
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
	add rsp, 8 ; (8 * argnum)
 ; function call
 	push rax

	movsd xmm0, [rsp] ; moving tmp variables from stack to xmm registers
	add rsp, 8
	movsd xmm1, [rsp]
	add rsp, 8
	mulsd xmm1, xmm0 ; multiplication
	sub rsp, 8
	movq [rsp], xmm1

	pop rax

	pop rbp
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rdx
	pop rcx
	pop rbx

	add rsp, 8 ; (8 * (lvn + an))

	ret

out:

	sub rsp, 72 ; (lvn + an)
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
	add rcx, 136; (pushed registers + lnvn +an - 1)
	mov rbp, rcx

	mov rbx, [rbp + 16] ; rbp + (an - n + 1) * 8 (if has arguments)
	mov [rbp + 0], rbx ; rbp + argnum

	mov r8, rcx

	mov rdi, rbp
	sub rdi, 1
	mov qword rax, [rbp]


	mov rbx, 01h
	mov rdx, rax
	xor rcx, rcx
	mov rcx, 64
	mov byte [rdi], 0x0A
	dec rdi
llPrintingLoop:
	mov rdx, rax
	cmp rcx, 0x00
	je llPrintingLoopEnd
	and rdx, rbx
	add rdx, 0x30
	mov byte [rdi], dl
	dec rdi
	shr rax, 1
	dec rcx
	jmp llPrintingLoop
llPrintingLoopEnd:
	mov rax, 0x01
	mov rsi, rdi
	mov rdi, 0x01
	mov rdx, 66
	syscall

	pop rbp
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rdx
	pop rcx
	pop rbx

	add rsp, 72 ; (8 * (lvn + an))

	ret

halt:
	mov rax, 0x3c
	syscall
