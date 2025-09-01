bits 64
default rel

global isr_stub_table
global isr_common_stub
extern isr_common_frame

section .text

isr_stub_table:
%assign v 0
%rep 256
    dq isr_stub%+v
%assign v v+1
%endrep

%macro ISR_NOERR 1
global isr_stub%1
isr_stub%1:
    push qword 0
    push qword %1
    jmp isr_common_stub
%endmacro

%macro ISR_ERR 1
global isr_stub%1
isr_stub%1:
    push qword %1
    jmp isr_common_stub
%endmacro

%assign v 0
%rep 256
%if v = 8 || v = 10 || v = 11 || v = 12 || v = 13 || v = 14 || v = 17 || v = 30
    ISR_ERR v
%else
    ISR_NOERR v
%endif
%assign v v+1
%endrep

isr_common_stub:
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rax, rsp
    mov rdi, rax

    and rsp, -16
    add rsp, 8
    call isr_common_frame
    mov rsp, rax

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax

    add rsp, 16
    iretq
