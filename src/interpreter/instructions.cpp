#include "instructions.h"
#include "parser/parser.h"
#include "interpreter.h"
#include "syscalls.h"

namespace Instructions
{

u32 lea(GlobalState& globalState, Instruction& instruction) {
    auto& operand = std::get<Memory>(instruction.operands[0]);
    u64 addr = resolveMemory(operand, globalState);
    writeOperand(instruction.operands[1], addr, globalState);
    return 0;
}

u32 Xor(GlobalState& globalState, Instruction& instruction) {
    auto srcName = std::get<Register>(instruction.operands[0]).name.substr(1);
    auto dstName = std::get<Register>(instruction.operands[1]).name.substr(1);

    auto operandSize = getOperandSize(instruction.operands[0], instruction.operands[1], globalState.cpu, instruction.mnemonic.suffix);
    u64 left = readOperand(instruction.operands[0], operandSize, globalState);
    u64 right = readOperand(instruction.operands[1], operandSize, globalState);
    writeOperand(instruction.operands[1], left ^ right, globalState);
    return 0;
}

u32 mov(GlobalState& globalState, Instruction& instruction) {
    auto operandSize = getOperandSize(instruction.operands[0], instruction.operands[1], globalState.cpu, instruction.mnemonic.suffix);
    u64 left = readOperand(instruction.operands[0], operandSize, globalState);
    writeOperand(instruction.operands[1], left, globalState);
    return 0;
}

u32 syscall(GlobalState& globalState, Instruction& instruction) {
    if (syscallTable.find(globalState.cpu.rax) == syscallTable.end()) {
        LOG_ERROR("Unknown syscall number {}", globalState.cpu.rax);
    }
    syscallTable[globalState.cpu.rax](globalState.cpu, globalState.memory);
    if (globalState.cpu.rax == 60) {
        return 1;
    }
    return 0;
}

} // namespace Instructions
