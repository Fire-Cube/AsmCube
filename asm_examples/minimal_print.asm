.section .rodata

msg0:
    .ascii  "If you see this, the test passed!\n"
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
    syscall

    mov $60, %rax
    syscall

    .cfi_endproc
.size _start, .-_start