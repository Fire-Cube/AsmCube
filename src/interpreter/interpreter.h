// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

#include "types.h"
#include "registers.h"
#include "parser/parser.h"
#include "testcases/loader.h"
#include "global_state.h"

u64 resolveMemory(const Memory& memory, GlobalState& globalState);
std::string getOperandSize(const Operand& left, const Operand& right, const CPU& cpu, const std::string& sizeSuffix);
u64 readOperand(const Operand& operand, std::string& targetSize, GlobalState& globalState);
void writeOperand(const Operand& operand, u64 value, GlobalState& globalState);
int run(Ast& ast, GlobalState& globalState);
