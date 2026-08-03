// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

#include "types.h"
#include "registers.h"
#include "parser/parser.h"
#include "testcases/loader.h"
#include "global_state.h"

namespace Interpreter
{

u64 resolveMemory(const Ast::Memory& memory, GlobalState& globalState);

std::string getOperandSize(const Ast::Operand& left, const CPU& cpu, const std::string& sizeSuffix);
std::string getOperandSize(const Ast::Operand& left, const Ast::Operand& right, const CPU& cpu, const std::string& sizeSuffix);
u64 readOperand(const Ast::Operand& operand, std::string& targetSize, GlobalState& globalState);
void writeOperand(const Ast::Operand& operand, u64 value, GlobalState& globalState);
int run(Ast::Ast& ast, GlobalState& globalState);

} // namespace Interpreter