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

    u64 width;
    switch (operandSize[0]) {
        case 'b': width = 8;  break;
        case 'w': width = 16; break;
        case 'l': width = 32; break;
        case 'q': width = 64; break;
    }

    u64 mask = (width == 64) ? ~0ULL : ((1ULL << width) - 1);
    u64 a = left & mask;
    u64 b = right & mask;

    u128 sum = static_cast<u128>(a) + static_cast<u128>(b);
    u64 res = static_cast<u64>(sum & mask);

    // CF
    globalState.cpu.cf = (width == 64) ? (sum > static_cast<u128>(UINT64_MAX)) : static_cast<s32>((sum >> width) & 1);

    // OF
    u64 sign = 1ULL << (width - 1);
    u64 sa = a & sign;
    u64 sb = b & sign;
    u64 sr = res & sign;
    globalState.cpu.of = (sa == sb) && (sa != sr);

    // SF
    globalState.cpu.sf = (res & sign) != 0;

    writeOperand(instruction.operands[1], sum, globalState);
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

u32 jmp(GlobalState& globalState, Instruction& instruction) {
    std::string targetSize = "q";
    globalState.cpu.rip = readOperand(instruction.operands[0], targetSize, globalState);
    return 0;
}

u32 Jcc(GlobalState& globalState, Instruction& instruction) {
    CondCode condCode = std::get<CondCode>(instruction.additionalData.value());

    bool shouldJump = false;
    switch (condCode) {
        case CondCode::overflow:
            shouldJump = globalState.cpu.of; break;
        case CondCode::notOverflow:
            shouldJump = !globalState.cpu.of; break;
        case CondCode::sign:
            shouldJump = globalState.cpu.sf; break;
        case CondCode::notSign:
            shouldJump = !globalState.cpu.sf; break;
        case CondCode::equal:
        case CondCode::zero:
            shouldJump = globalState.cpu.zf; break;
        case CondCode::notEqual:
        case CondCode::notZero:
            shouldJump = !globalState.cpu.zf; break;
        case CondCode::below:
        case CondCode::notAboveOrEqual:
        case CondCode::carry:
            shouldJump = globalState.cpu.cf; break;
        case CondCode::notBelow:
        case CondCode::aboveOrEqual:
        case CondCode::notCarry:
            shouldJump = !globalState.cpu.cf; break;
        case CondCode::belowOrEqual:
        case CondCode::notAbove:
            shouldJump = globalState.cpu.cf ||globalState.cpu.zf; break;
        case CondCode::above:
        case CondCode::notBelowOrEqual:
            shouldJump = !globalState.cpu.cf && !globalState.cpu.zf; break;
        case CondCode::less:
        case CondCode::notGreaterOrEqual:
            shouldJump = globalState.cpu.sf != globalState.cpu.of; break;
        case CondCode::greaterOrEqual:
        case CondCode::notLess:
            shouldJump = globalState.cpu.sf == globalState.cpu.of; break;
        case CondCode::lessOrEqual:
        case CondCode::notGreater:
            shouldJump = globalState.cpu.zf || (globalState.cpu.sf != globalState.cpu.of); break;
        case CondCode::greater:
        case CondCode::notLessOrEqual:
            shouldJump = !globalState.cpu.zf && (globalState.cpu.sf == globalState.cpu.of); break;
        case CondCode::parity:
        case CondCode::parityEven:
            shouldJump = globalState.cpu.pf; break;
        case CondCode::notParity:
        case CondCode::parityOdd:
            shouldJump = !globalState.cpu.pf; break;
    }

    if (shouldJump) {
        std::string targetSize = "q";
        u64 targetAdress = readOperand(instruction.operands[0], targetSize, globalState);
        globalState.cpu.rip = targetAdress;
    }
    else {
        globalState.cpu.rip += 8;
    }

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
