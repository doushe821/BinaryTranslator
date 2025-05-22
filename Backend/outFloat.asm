PrintArgF:
    push rax
    push rbx
    push rdx

    mov rbx, DOUBLE_SIZE
    call CheckSign
    push rcx

    cmp rdx, FLAG_MINUS
    je .NegativeNum

    mov rdx, rax
    shr rdx, MANTISSA_LEN
    mov rbx, EXPONENT
    inc rdx
    sub rdx, rbx                             ; DX - Exponent

.ContinueNegativeNum:
    mov rbx, rax
    shl rbx, DOUBLE_SIZE - MANTISSA_LEN
    shr rbx, DOUBLE_SIZE - MANTISSA_LEN
    add rbx, qword [MANTISSA_ONE]                    ; RBX - Mantissa

    tzcnt rcx, rbx                                   ; RCX = Number of trailing zeroes in RBX
    shr rbx, cl

    push rbx
    mov rbx, rcx
    mov rcx, MANTISSA_LEN
    sub rcx, rbx
    sub rcx, rdx
    pop rbx

                                                ; Printed number = RBX * 2 ^ (-RCX)
                                                ; Printed number = RBX * 5 ^ (RCX) * 10 ^ (-RCX)
    cmp rcx, [SIGN_MASK]
    ja .NegRCX
    je .ZeroRCX
    cmp rcx, 0x0
    je .ZeroRCX

    push rax
    push rdx

    lzcnt rdx, rbx                          ; RDX = Number of leading zeroes in RBX
    push rcx                                ; Push old RCX
    shl rcx, 1                              ; RCX = old RCX * 2
    pop rax                                 ; RAX = old RCX
    add rcx, rax                            ; RCX = old RCX * 3
.ConditionalRoundResult:
    cmp rdx, rcx                            ; What is greater?
                                            ; Number of leading zeroes in RBX or 3 * RCX
    jae .StopRounding

    sub rcx, rdx
    shr rcx, 2                              ; RCX - RDX = (old RCX - new RCX) / 3 + (new RDX - old RDX)
    inc rcx
    shr rbx, cl
    sub rax, rcx

.StopRounding:
    mov rcx, rax

    push rcx
.For:
    imul rbx, FIVE                          ; 5 = 10 / 2
    loop .For
    pop rcx

    pop rdx
    pop rax

    mov rax, rbx
    mov rbx, rcx
    pop rcx
    call PrintNumber

.Done:
    pop rdx
    pop rbx
    pop rax

.Skip:
    ret

.NegRCX:
    neg rcx
    shl rbx, cl
    mov rax, rbx
    pop rcx
    call PrintArgD
    jmp .Done

.ZeroRCX:
    mov rax, rbx
    pop rcx
    call PrintArgD
    jmp .Done

.NegativeNum:
    neg rax                                  ; In function of checking sign RAX was negative
    shl rax, 1
    shr rax, 1
    mov rdx, rax
    shr rdx, MANTISSA_LEN
    mov rbx, EXPONENT
    inc rdx
    sub rdx, rbx                             ; DX - Exponent
    jmp .ContinueNegativeNum

CheckSign:

    push rax

    push rcx
    mov rcx, rbx
    sub rcx, 1
    shr rax, cl
    pop rcx

    cmp rax, 1
    je .Minus
    pop rax
    mov rdx, FLAG_PLUS

.Done:
    ret

.Minus:
    cmp rcx, BUFFER_LEN
    je .BufferEnd

.Continue:
    pop rax
    neg rax
    mov byte [Buffer + rcx],  MINUS
    inc rcx
    mov rdx, FLAG_MINUS
    jmp .Done

.BufferEnd:
    push rcx
    call PrintBuffer
    pop rcx
    jmp .Continue