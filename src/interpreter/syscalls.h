// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <unordered_map>
#include <functional>

#include "registers.h"
#include "memory.h"

void syscall_read(CPU& cpu, Interpreter::Memory& memory);
void syscall_write(CPU& cpu, Interpreter::Memory& memory);
void syscall_open(CPU& cpu, Interpreter::Memory& memory);
void syscall_exit(CPU& cpu, Interpreter::Memory& memory);

inline std::unordered_map<u32, std::function<void(CPU& cpu, Interpreter::Memory& memory)>> syscallTable = {
    {0,  syscall_read},
    {1,  syscall_write},
    {2,  syscall_open},
    {60, syscall_exit},
};
