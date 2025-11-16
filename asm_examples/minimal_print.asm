.section .rodata

msg0:
    .ascii  "Syntax Test\n-----------\n"
msglen0 = . - msg0

.section .text

.global _start

.type _start, @function
_start:
    .cfi_startproc
    .cfi_undefined rip

    mov $1, %rax
    mov $1, %rdi
    lea msg0(%rip), %rsi
    mov $msglen0, %rdx
    mov (%rsi), %rsi
    syscall

    xor %rdi, %rdi
    syscall

    .cfi_endproc
.size _start, .-_start