.section .text

.global _start
_start:
    mov $5, %rax
    mov $2, %rbx
    sub %rbx, %rax
    checkpoint $1

    mov $0, %rax
    mov $0, %rbx
    sub %rbx, %rax
    checkpoint $2

    mov $0, %rax
    mov $1, %rbx
    sub %rbx, %rax
    checkpoint $3

    mov $0x8000000000000000, %rax
    mov $1, %rbx
    sub %rbx, %rax
    checkpoint $4

    mov $0x7fffffffffffffff, %rax
    mov $0xffffffffffffffff, %rbx
    sub %rbx, %rax
    checkpoint $5

    mov $0, %rax
    mov $2, %rbx
    sub %rbx, %rax
    checkpoint $6
