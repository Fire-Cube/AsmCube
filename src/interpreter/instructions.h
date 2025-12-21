// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "interpreter.h"
#include "parser/parser.h"

namespace Instructions {

u32 lea(GlobalState& globalState, Instruction& instruction);
u32 Xor(GlobalState& globalState, Instruction& instruction);
u32 add(GlobalState& globalState, Instruction& instruction);
u32 sub(GlobalState& globalState, Instruction& instruction);
u32 mov(GlobalState& globalState, Instruction& instruction);
u32 push(GlobalState& globalState, Instruction& instruction);
u32 pop(GlobalState& globalState, Instruction& instruction);
u32 call(GlobalState& globalState, Instruction& instruction);
u32 ret(GlobalState& globalState, Instruction& instruction);
u32 hlt(GlobalState& globalState, Instruction& instruction);
u32 leave(GlobalState& globalState, Instruction& instruction);
u32 syscall(GlobalState& globalState, Instruction& instruction);

} // namespace Instructions
