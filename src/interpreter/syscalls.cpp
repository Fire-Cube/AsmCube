#include <string>

#include "syscalls.h"

void syscall_read(CPU& cpu, Interpreter::Memory& memory) {
    u32 fd = static_cast<u32>(cpu.rdi);
    u64 bufAddress = cpu.rsi;
    u32 count = static_cast<u32>(cpu.rdx);

    if (fd != 0) {
        LOG_ERROR("Unsupported file descriptor {} in syscall_read", fd);
    }

    std::string input;
    if (fd == 0) {
        std::getline(std::cin, input);
    }
    for (u32 i = 0; i < count - 1; ++i) {
        if (i > input.size()) {
            break;
        }
        u8 byte = static_cast<u8>(input[i]);
        memory.writeMemory<u8>(bufAddress + i, byte);
    }
    cpu.rax = input.size();
}

void syscall_write(CPU& cpu, Interpreter::Memory& memory) {
    u32 fd = static_cast<u32>(cpu.rdi);
    u64 bufAddress = cpu.rsi;
    u32 count = static_cast<u32>(cpu.rdx);
    if (fd != 1 && fd != 2) {
        LOG_ERROR("Unsupported file descriptor {} in syscall_write", fd);
    }

    for (u32 i = 0; i < count; ++i) {
        u8 byte = 0;
        memory.readMemory<u8>(bufAddress + i, byte);
        if (fd == 1) {
            std::cout << static_cast<char>(byte);
        }
        else if (fd == 2) {
            std::cerr << static_cast<char>(byte);
        }
    }
}

void syscall_exit(CPU& cpu, Interpreter::Memory& memory) {
    u32 exitCode = static_cast<u32>(cpu.rdi);
    LOG_INFO("Program finished with exit code {}", exitCode);
}