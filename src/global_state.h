// SPDX-FileCopyrightText: Copyright 2026 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

#include "interpreter/symbol_table.h"
#include "interpreter/memory.h"
#include "interpreter/registers.h"
#include "testcases/testcase.h"

struct GlobalState {
    CPU cpu{};
    Interpreter::Memory memory{};
    SymbolTable symbolTable{};
    std::vector<SymbolImmediate> symbolImmediates{};
    Testcases::Test testcase{};
};
