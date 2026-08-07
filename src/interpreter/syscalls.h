// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <unordered_map>
#include <functional>

#include "registers.h"
#include "memory.h"

namespace Interpreter::Syscalls
{

void syscall_read(CPU& cpu, Memory& memory);
void syscall_write(CPU& cpu, Memory& memory);
void syscall_open(CPU& cpu, Memory& memory);
void syscall_exit(CPU& cpu, Memory& memory);

inline std::unordered_map<u32, void (*)(CPU&, Memory&)> syscallTable = {
    {0,  syscall_read},
    {1,  syscall_write},
    {2,  syscall_open},
    {60, syscall_exit},
};

} // namespace Interpreter::Syscalls