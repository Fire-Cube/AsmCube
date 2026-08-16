// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <string>

#ifdef _WIN32
    #include <io.h>
    #define read  _read
    #define write _write
    #define open _open
#else
    #include <unistd.h>
    #include <fcntl.h>
#endif

#include "syscalls.h"

namespace Interpreter::Syscalls
{

void syscall_read(CPU& cpu, Memory& memory) {
    s32 fd = static_cast<u32>(cpu.rdi);
    u64 bufAddress = cpu.rsi;
    u64 count = static_cast<u32>(cpu.rdx);

    void* inputRaw = malloc(count);

    s64 readBytesCount = read(fd, inputRaw, count);

    u8* input = static_cast<u8*>(inputRaw);
    for (u32 i = 0; i < readBytesCount; ++i) {
        memory.writeMemory<u8>(bufAddress + i, input[i]);
    }

    free(inputRaw);
    cpu.rax = readBytesCount;
}

void syscall_write(CPU& cpu, Memory& memory) {
    s32 fd = static_cast<u32>(cpu.rdi);
    u64 bufAddress = cpu.rsi;
    u64 count = static_cast<u32>(cpu.rdx);

    auto* outputRaw = static_cast<u8*>(malloc(count));
    for (u32 i = 0; i < count; ++i) {
        memory.readMemory<u8>(bufAddress + i, outputRaw[i]);
    }
    s64 writtenBytesCount = write(fd, outputRaw, count);
    free(outputRaw);

    cpu.rax = writtenBytesCount;
}

void syscall_open(CPU& cpu, Memory& memory) {
    u64 pathAddress = (cpu.rdi);
    u32 flags = static_cast<u32>(cpu.rsi); // Needs mapping from Linux to Windows
    u32 mode = static_cast<u32>(cpu.rdx);

    std::string path;
    u8 byte = 1;
    u64 offset = 0;
    while (byte != 0) {
        memory.readMemory<u8>(pathAddress + offset, byte);
        path += static_cast<char>(byte);
        ++offset;
    };
    s32 result = open(path.c_str(), flags, mode);
    cpu.rax = result;
}

void syscall_exit(CPU& cpu, Memory& memory) {
    u32 exitCode = static_cast<u32>(cpu.rdi);
    LOG_INFO("Program finished with exit code {}", exitCode);
}

void syscall_getrandom(CPU& cpu, Memory& memory) {
    u64 bufAddress = cpu.rdi;
    u32 count = static_cast<u32>(cpu.rsi);
    u32 flags = static_cast<u32>(cpu.rdx);
}

} // namespace Interpreter::Syscalls