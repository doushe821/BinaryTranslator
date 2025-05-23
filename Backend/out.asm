out:
pop rax
sub rsp, 64
push rbp
mov rbp, rsp
add rbp, 72
push rbx
push rcx
push rdx
push rdi
push rsi
push r9
mov r9, rax
mov rdi, rbp
sub rdi, 8
mov qword rax, [rbp]
mov rbx, 01h
mov rdx, rax
mov rcx, 64
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
mov rdx, 64
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
add rsp, 56
ret