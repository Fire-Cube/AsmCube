#pragma once

#include "interpreter.h"
#include "parser/parser.h"

namespace Instructions {

u32 lea(GlobalState& globalState, Instruction& instruction);
u32 Xor(GlobalState& globalState, Instruction& instruction);
u32 add(GlobalState& globalState, Instruction& instruction);
u32 mov(GlobalState& globalState, Instruction& instruction);
u32 syscall(GlobalState& globalState, Instruction& instruction);

} // namespace Instructions