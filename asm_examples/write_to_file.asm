.section .rodata
path:
    .ascii "out.txt\0"

text:
    .ascii "This is a sample text!\n"
textlen = . - text

.section .text
.global _start
_start:
    mov     $2, %rax
    lea     path(%rip), %rdi
    mov     $769, %rsi # need to change to Linux flags, when mapping is implemented
    mov     $0644, %rdx
    syscall

    mov     %rax, %rdi
    mov     $1, %rax
    lea     text(%rip), %rsi
    mov     $textlen, %rdx
    syscall

    mov     $60, %rax
    xor     %rdi, %rdi
    syscall
