// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "parser/parser.h"
#include "global_state.h"

namespace Interpreter::Instructions {

u32 lea(GlobalState& globalState, Ast::Instruction& instruction);
u32 Xor(GlobalState& globalState, Ast::Instruction& instruction);
u32 And(GlobalState& globalState, Ast::Instruction& instruction);
u32 add(GlobalState& globalState, Ast::Instruction& instruction);
u32 sub(GlobalState& globalState, Ast::Instruction& instruction);
u32 cmp(GlobalState& globalState, Ast::Instruction& instruction);
u32 inc(GlobalState& globalState, Ast::Instruction& instruction);
u32 dec(GlobalState& globalState, Ast::Instruction& instruction);
u32 neg(GlobalState& globalState, Ast::Instruction& instruction);
u32 test(GlobalState& globalState, Ast::Instruction& instruction);
u32 stc(GlobalState& globalState, Ast::Instruction& instruction);
u32 mov(GlobalState& globalState, Ast::Instruction& instruction);
u32 push(GlobalState& globalState, Ast::Instruction& instruction);
u32 pop(GlobalState& globalState, Ast::Instruction& instruction);
u32 call(GlobalState& globalState, Ast::Instruction& instruction);
u32 ret(GlobalState& globalState, Ast::Instruction& instruction);
u32 jmp(GlobalState& globalState, Ast::Instruction& instruction);
u32 Jcc(GlobalState& globalState, Ast::Instruction& instruction);
u32 CMOVcc(GlobalState& globalState, Ast::Instruction& instruction);
u32 hlt(GlobalState& globalState, Ast::Instruction& instruction);
u32 leave(GlobalState& globalState, Ast::Instruction& instruction);
u32 syscall(GlobalState& globalState, Ast::Instruction& instruction);
u32 checkpoint(GlobalState& globalState, Ast::Instruction& instruction);

} // namespace Interpreter::Instructions
