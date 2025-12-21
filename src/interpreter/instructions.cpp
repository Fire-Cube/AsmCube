// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

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
    globalState.cpu.rip += 8;
    return 0;
}

u32 Xor(GlobalState& globalState, Instruction& instruction) {
    auto operandSize = getOperandSize(instruction.operands[0], instruction.operands[1], globalState.cpu, instruction.mnemonic.suffix);
    u64 left = readOperand(instruction.operands[0], operandSize, globalState);
    u64 right = readOperand(instruction.operands[1], operandSize, globalState);
    writeOperand(instruction.operands[1], left ^ right, globalState);
    globalState.cpu.rip += 8;
    return 0;
}

u32 add(GlobalState& globalState, Instruction& instruction) {
    auto operandSize = getOperandSize(instruction.operands[0], instruction.operands[1], globalState.cpu, instruction.mnemonic.suffix);
    u64 left = readOperand(instruction.operands[0], operandSize, globalState);
    u64 right = readOperand(instruction.operands[1], operandSize, globalState);
    writeOperand(instruction.operands[1], left + right, globalState);
    globalState.cpu.rip += 8;
    return 0;
}

u32 sub(GlobalState& globalState, Instruction& instruction) {
    auto operandSize = getOperandSize(instruction.operands[0], instruction.operands[1], globalState.cpu, instruction.mnemonic.suffix);
    u64 left = readOperand(instruction.operands[0], operandSize, globalState);
    u64 right = readOperand(instruction.operands[1], operandSize, globalState);
    writeOperand(instruction.operands[1], left - right, globalState);
    globalState.cpu.rip += 8;
    return 0;
}

u32 mov(GlobalState& globalState, Instruction& instruction) {
    auto operandSize = getOperandSize(instruction.operands[0], instruction.operands[1], globalState.cpu, instruction.mnemonic.suffix);
    u64 left = readOperand(instruction.operands[0], operandSize, globalState);
    writeOperand(instruction.operands[1], left, globalState);
    globalState.cpu.rip += 8;
    return 0;
}

u32 push(GlobalState& globalState, Instruction& instruction) {
    auto operandSize = getOperandSize(instruction.operands[0], instruction.operands[0], globalState.cpu, instruction.mnemonic.suffix);
    u64 value = readOperand(instruction.operands[0], operandSize, globalState);
    globalState.cpu.rsp -= 8;
    globalState.memory.writeMemoryNoExcept(globalState.cpu.rsp, value);
    globalState.cpu.rip += 8;
    return 0;
}

u32 pop(GlobalState& globalState, Instruction& instruction) {
    u64 value;
    globalState.memory.readMemoryNoExcept(globalState.cpu.rsp, value);
    writeOperand(instruction.operands[0], value, globalState);
    globalState.cpu.rsp += 8;
    globalState.cpu.rip += 8;
    return 0;
}

u32 call(GlobalState& globalState, Instruction& instruction) {
    std::string targetSize = "q";
    u64 address = readOperand(instruction.operands[0], targetSize, globalState);
    globalState.cpu.rsp -= 8;
    globalState.memory.writeMemoryNoExcept(globalState.cpu.rsp, globalState.cpu.rip);
    globalState.cpu.rip = address;
    return 0;
}

u32 ret(GlobalState& globalState, Instruction& instruction) {
    u64 returnAddress;
    globalState.memory.readMemoryNoExcept(globalState.cpu.rsp, returnAddress);
    globalState.cpu.rsp += 8;
    globalState.cpu.rip = returnAddress + 8;
    return 0;
}

u32 hlt(GlobalState& globalState, Instruction& instruction) {
    LOG_INFO("HLT encountered. Halting execution.");
    std::exit(0);
}

u32 leave(GlobalState& globalState, Instruction& instruction) {
    globalState.cpu.rsp = globalState.cpu.rbp;
    u64 oldRbp;
    globalState.memory.readMemoryNoExcept(globalState.cpu.rsp, oldRbp);
    globalState.cpu.rbp = oldRbp;
    globalState.cpu.rsp += 8;
    globalState.cpu.rip += 8;
    return 0;
}

u32 syscall(GlobalState& globalState, Instruction& instruction) {
    if (syscallTable.find(globalState.cpu.rax) == syscallTable.end()) {
        LOG_ERROR("Unknown syscall number {}", globalState.cpu.rax);
    }
    syscallTable[globalState.cpu.rax](globalState.cpu, globalState.memory);
    globalState.cpu.rip += 8;
    if (globalState.cpu.rax == 60) {
        return 1;
    }
    return 0;
}

} // namespace Instructions
