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

Ast::Width getOperandSize(const Ast::Operand& left, std::optional<Ast::Width> suffix);
Ast::Width getOperandSize(const Ast::Operand& left, const Ast::Operand& right, std::optional<Ast::Width> suffix);
u64 readOperand(const Ast::Operand& operand, Ast::Width targetSize, GlobalState& globalState);
void writeOperand(const Ast::Operand& operand, u64 value, Ast::Width targetSize, GlobalState& globalState);
int run(Ast::Ast& ast, GlobalState& globalState);

} // namespace Interpreter