.section .bss

input_buffer:
    .skip 100

.section .rodata

newline:
    .ascii "\n"

msg:
    .ascii "Please enter some text: "
msglen = . - msg

.section .text

.global _start

_start:
    mov     $msglen, %rdx
    lea     msg(%rip), %rsi
    mov     $1, %rdi
    mov     $1, %rax
    syscall

    mov     $10, %rdx
    lea     input_buffer(%rip), %rsi
    mov     $0, %rax
    mov     $0, %rdi
    syscall

    mov     %rax, %rdx
    lea     input_buffer(%rip), %rsi
    mov     $1, %rdi
    mov     $1, %rax
    syscall

    mov     $1, %rdx
    lea     newline(%rip), %rsi
    mov     $1, %rdi
    mov     $1, %rax
    syscall

    mov $60, %rax
    xor %rdi, %rdi
    syscall
