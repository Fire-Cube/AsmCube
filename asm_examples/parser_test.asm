hello_world:
    mov $1, %rax
    mov $1, %rdi
    lea hello_str(%rip), %rsi
    mov $hello_len, %rdx
    syscall
    ret

.global _start
_start:
    lea     dat0(%rip), %r11

    mov     $8, %r8
    mov     $16, %r9
    mov     $8, %r10
    mov     (%r11, %r8), %rcx
    mov     (%r11, %r9), %rdx
    mov     4(%r11, %r10,2), %rbx
    mov     -8(%r11, %r8), %r12

    call    hello_world
    call    hello_world

    mov     $123456, %rax
    mov     %rax, -8(%rsp)
    mov     -8(%rsp), %rax

    mov     $0x3a, %rax

    mov     $60, %r12
    push    %r12
    pop     %rax
    xor     %rdi, %rdi
    syscall

.section .rodata

dat0:
    .quad 1234
    .quad 5678
    .quad 9101112
    .byte 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
    .byte 0xff

hello_str:
    .ascii "Hello, World!\n"
hello_len = . - hello_str

.section .rodata.str1.1

.bss
dat1:
    .skip 512

.text
