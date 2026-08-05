// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include "instructions.h"
#include "instructions_helper.h"
#include "parser/parser.h"
#include "interpreter.h"
#include "syscalls.h"
#include "testcases/testcase.h"

namespace Interpreter::Instructions
{
u32 lea(GlobalState& globalState, Ast::Instruction& instruction) {
    auto& operand = std::get<Ast::Memory>(instruction.operands[0]);
    u64 addr = resolveMemory(operand, globalState);
    std::string targetSize = "q";
    writeOperand(instruction.operands[1], addr, targetSize, globalState);
    globalState.cpu.rip += 8;
    return 0;
}

u32 Xor(GlobalState& globalState, Ast::Instruction& instruction) {
    auto operandSize = getOperandSize(instruction.operands[0], instruction.operands[1], globalState.cpu, instruction.mnemonic.suffix);
    u64 left = readOperand(instruction.operands[0], operandSize, globalState);
    u64 right = readOperand(instruction.operands[1], operandSize, globalState);

    u64 width = Helper::getSizeOfGPR(operandSize[0]);

    u64 mask = (width == 64) ? ~0ULL : ((1ULL << width) - 1);
    u64 a = left & mask;
    u64 b = right & mask;

    u128 value = static_cast<u128>(a) ^ static_cast<u128>(b);
    u64 res = static_cast<u64>(value & mask);

    // CF
    globalState.cpu.cf = 0;

    // OF
    globalState.cpu.of = 0;

    // SF, ZF, PF
    Helper::calculateFlagsSFZFPF(globalState, res, width);

    writeOperand(instruction.operands[1], res, operandSize, globalState);
    globalState.cpu.rip += 8;
    return 0;
}

u32 And(GlobalState& globalState, Ast::Instruction& instruction) {
    auto operandSize = getOperandSize(instruction.operands[0], instruction.operands[1], globalState.cpu, instruction.mnemonic.suffix);
    u64 left = readOperand(instruction.operands[0], operandSize, globalState);
    u64 right = readOperand(instruction.operands[1], operandSize, globalState);

    u64 width = Helper::getSizeOfGPR(operandSize[0]);

    u64 mask = (width == 64) ? ~0ULL : ((1ULL << width) - 1);
    u64 a = left & mask;
    u64 b = right & mask;

    u128 value = static_cast<u128>(a) & static_cast<u128>(b);
    u64 res = static_cast<u64>(value & mask);

    // CF
    globalState.cpu.cf = 0;

    // OF
    globalState.cpu.of = 0;

    // SF, ZF, PF
    Helper::calculateFlagsSFZFPF(globalState, res, width);

    writeOperand(instruction.operands[1], res, operandSize, globalState);
    globalState.cpu.rip += 8;
    return 0;
}

u32 add(GlobalState& globalState, Ast::Instruction& instruction) {
    auto operandSize = getOperandSize(instruction.operands[0], instruction.operands[1], globalState.cpu, instruction.mnemonic.suffix);
    u64 left = readOperand(instruction.operands[0], operandSize, globalState);
    u64 right = readOperand(instruction.operands[1], operandSize, globalState);

    u64 width = Helper::getSizeOfGPR(operandSize[0]);

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

    // SF, ZF, PF
    Helper::calculateFlagsSFZFPF(globalState, res, width);

    writeOperand(instruction.operands[1], res, operandSize, globalState);
    globalState.cpu.rip += 8;
    return 0;
}

u32 sub(GlobalState& globalState, Ast::Instruction& instruction) {
    auto operandSize = getOperandSize(instruction.operands[0], instruction.operands[1], globalState.cpu, instruction.mnemonic.suffix);
    u64 left = readOperand(instruction.operands[0], operandSize, globalState);
    u64 right = readOperand(instruction.operands[1], operandSize, globalState);

    u64 width = Helper::getSizeOfGPR(operandSize[0]);

    u64 mask = (width == 64) ? ~0ULL : ((1ULL << width) - 1);
    u64 a = left & mask;
    u64 b = right & mask;

    u128 diff = static_cast<u128>(b) - static_cast<u128>(a);
    u64 res = static_cast<u64>(diff) & mask;

    // CF
    globalState.cpu.cf = (b < a);

    // OF
    u64 sign = 1ULL << (width - 1);
    u64 sb = b & sign;
    u64 sa = a & sign;
    u64 sr = res & sign;
    globalState.cpu.of = (sb != sa) && (sr != sb);

    // SF, ZF, PF
    Helper::calculateFlagsSFZFPF(globalState, res, width);

    writeOperand(instruction.operands[1], res, operandSize, globalState);
    globalState.cpu.rip += 8;
    return 0;
}

u32 cmp(GlobalState& globalState, Ast::Instruction& instruction) {
    // CMP is basically SUB without writing the result
    auto operandSize = getOperandSize(instruction.operands[0], instruction.operands[1], globalState.cpu, instruction.mnemonic.suffix);
    u64 left = readOperand(instruction.operands[0], operandSize, globalState);
    u64 right = readOperand(instruction.operands[1], operandSize, globalState);

    u64 width = Helper::getSizeOfGPR(operandSize[0]);

    u64 mask = (width == 64) ? ~0ULL : ((1ULL << width) - 1);
    u64 a = left & mask;
    u64 b = right & mask;

    u128 diff = static_cast<u128>(b) - static_cast<u128>(a);
    u64 res = static_cast<u64>(diff) & mask;

    // CF
    globalState.cpu.cf = (b < a);

    // OF
    u64 sign = 1ULL << (width - 1);
    u64 sb = b & sign;
    u64 sa = a & sign;
    u64 sr = res & sign;
    globalState.cpu.of = (sb != sa) && (sr != sb);

    // SF, ZF, PF
    Helper::calculateFlagsSFZFPF(globalState, res, width);

    globalState.cpu.rip += 8;
    return 0;
}

u32 inc(GlobalState& globalState, Ast::Instruction& instruction) {
    auto operandSize = getOperandSize(instruction.operands[0], globalState.cpu, instruction.mnemonic.suffix);
    u64 width = Helper::getSizeOfGPR(operandSize[0]);
    u64 left = readOperand(instruction.operands[0], operandSize, globalState);

    u64 mask = (width == 64) ? ~0ULL : ((1ULL << width) - 1);

    u64 a = left & mask;

    u128 sum = static_cast<u128>(a) + 1;
    u64 res = static_cast<u64>(sum) & mask;

    // OF
    u64 sign = 1ULL << (width - 1);
    u64 sa = a & sign;
    u64 sr = res & sign;
    globalState.cpu.of = (sa == 0) && (sr != 0);

    // SF, ZF, PF
    Helper::calculateFlagsSFZFPF(globalState, res, width);

    globalState.cpu.rip += 8;

    writeOperand(instruction.operands[0], res, operandSize, globalState);
    return 0;
}

u32 dec(GlobalState& globalState, Ast::Instruction& instruction) {
    auto operandSize = getOperandSize(instruction.operands[0], globalState.cpu, instruction.mnemonic.suffix);
    u64 width = Helper::getSizeOfGPR(operandSize[0]);

    u64 left = readOperand(instruction.operands[0], operandSize, globalState);

    u64 mask = (width == 64) ? ~0ULL : ((1ULL << width) - 1);

    u64 a = left & mask;

    u128 diff = static_cast<u128>(a) - 1;
    u64 res = static_cast<u64>(diff) & mask;

    // OF
    u64 sign = 1ULL << (width - 1);
    u64 sa = a & sign;
    u64 sr = res & sign;
    globalState.cpu.of = (sa != 0) && (sr == 0);

    // SF, ZF, PF
    Helper::calculateFlagsSFZFPF(globalState, res, width);

    globalState.cpu.rip += 8;

    writeOperand(instruction.operands[0], res, operandSize, globalState);
    return 0;
}

u32 neg(GlobalState& globalState, Ast::Instruction& instruction) {
    auto operandSize = getOperandSize(instruction.operands[0], globalState.cpu, instruction.mnemonic.suffix);
    u64 width = Helper::getSizeOfGPR(operandSize[0]);

    u64 left = readOperand(instruction.operands[0], operandSize, globalState);

    u64 mask = (width == 64) ? ~0ULL : ((1ULL << width) - 1);

    u64 a = left & mask;

    u128 diff = 0 - static_cast<u128>(a);
    u64 res = static_cast<u64>(diff) & mask;

    // CF
    globalState.cpu.cf = (a != 0);

    // OF
    u64 sign = 1ULL << (width - 1);
    globalState.cpu.of = (a == sign);

    // SF, ZF, PF
    Helper::calculateFlagsSFZFPF(globalState, res, width);

    writeOperand(instruction.operands[0], res, operandSize, globalState);
    globalState.cpu.rip += 8;
    return 0;
}

u32 test(GlobalState& globalState, Ast::Instruction& instruction) {
    auto operandSize = getOperandSize(instruction.operands[0], globalState.cpu, instruction.mnemonic.suffix);
    u64 width = Helper::getSizeOfGPR(operandSize[0]);

    u64 left = readOperand(instruction.operands[0], operandSize, globalState);
    u64 right = readOperand(instruction.operands[1], operandSize, globalState);

    u64 result = left & right;

    // CF
    globalState.cpu.cf = 0;

    // OF
    globalState.cpu.of = 0;

    // SF, ZF, PF
    Instructions::Helper::calculateFlagsSFZFPF(globalState, result, width);

    globalState.cpu.rip += 8;
    return 0;
}

u32 stc(GlobalState& globalState, Ast::Instruction& instruction) {
    globalState.cpu.cf = 1;
    globalState.cpu.rip += 8;
    return 0;
}

u32 mov(GlobalState& globalState, Ast::Instruction& instruction) {
    auto operandSize = getOperandSize(instruction.operands[0], instruction.operands[1], globalState.cpu, instruction.mnemonic.suffix);
    u64 left = readOperand(instruction.operands[0], operandSize, globalState);
    writeOperand(instruction.operands[1], left, operandSize, globalState);
    globalState.cpu.rip += 8;
    return 0;
}

u32 push(GlobalState& globalState, Ast::Instruction& instruction) {
    auto operandSize = getOperandSize(instruction.operands[0], instruction.operands[0], globalState.cpu, instruction.mnemonic.suffix);
    u64 value = readOperand(instruction.operands[0], operandSize, globalState);
    globalState.cpu.rsp -= 8;
    globalState.memory.writeMemory(globalState.cpu.rsp, value);
    globalState.cpu.rip += 8;
    return 0;
}

u32 pop(GlobalState& globalState, Ast::Instruction& instruction) {
    u64 value;
    globalState.memory.readMemory(globalState.cpu.rsp, value);
    std::string targetSize = "q";
    writeOperand(instruction.operands[0], value, targetSize, globalState);
    globalState.cpu.rsp += 8;
    globalState.cpu.rip += 8;
    return 0;
}

u32 call(GlobalState& globalState, Ast::Instruction& instruction) {
    std::string targetSize = "q";
    u64 address = readOperand(instruction.operands[0], targetSize, globalState);
    globalState.cpu.rsp -= 8;
    globalState.memory.writeMemory(globalState.cpu.rsp, globalState.cpu.rip);
    globalState.cpu.rip = address;
    return 0;
}

u32 ret(GlobalState& globalState, Ast::Instruction& instruction) {
    u64 returnAddress;
    globalState.memory.readMemory(globalState.cpu.rsp, returnAddress);
    globalState.cpu.rsp += 8;
    globalState.cpu.rip = returnAddress + 8;
    return 0;
}

u32 jmp(GlobalState& globalState, Ast::Instruction& instruction) {
    std::string targetSize = "q";
    Ast::Operand& operand = instruction.operands[0];
    globalState.cpu.rip = readOperand(operand, targetSize, globalState);

    return 0;
}

u32 Jcc(GlobalState& globalState, Ast::Instruction& instruction) {
    auto condCode = std::get<Ast::CondCode>(instruction.additionalData.value());

    if (bool shouldJump = Helper::evaluateCondCodes(condCode, globalState)) {
        std::string targetSize = "q";
        Ast::Operand& operand = instruction.operands[0];
        u64 targetAdress = readOperand(operand, targetSize, globalState);
        globalState.cpu.rip = targetAdress;
    }
    else {
        globalState.cpu.rip += 8;
    }

    return 0;
}

u32 CMOVcc(GlobalState& globalState, Ast::Instruction& instruction) {
    auto condCode = std::get<Ast::CondCode>(instruction.additionalData.value());
    LOG_DEBUG("CondCode of CMOVcc is {}", magic_enum::enum_name(condCode));

    if (bool shouldMov = Helper::evaluateCondCodes(condCode, globalState)) {
        mov(globalState, instruction);
    }
    else {
        globalState.cpu.rip += 8;
    }
    return 0;
}

u32 hlt(GlobalState& globalState, Ast::Instruction& instruction) {
    LOG_INFO("HLT encountered. Halting execution.");
    return 1;
}

u32 leave(GlobalState& globalState, Ast::Instruction& instruction) {
    globalState.cpu.rsp = globalState.cpu.rbp;
    u64 oldRbp;
    globalState.memory.readMemoryNoExcept(globalState.cpu.rsp, oldRbp);
    globalState.cpu.rbp = oldRbp;
    globalState.cpu.rsp += 8;
    globalState.cpu.rip += 8;
    return 0;
}

u32 syscall(GlobalState& globalState, Ast::Instruction& instruction) {
    if (Syscalls::syscallTable.find(globalState.cpu.rax) == Syscalls::syscallTable.end()) {
        LOG_ERROR("Unknown syscall number {}", globalState.cpu.rax);
    }
    Syscalls::syscallTable[globalState.cpu.rax](globalState.cpu, globalState.memory);
    globalState.cpu.rip += 8;
    if (globalState.cpu.rax == 60) {
        return 1;
    }
    return 0;
}

u32 checkpoint(GlobalState& globalState, Ast::Instruction& instruction) {
    if (globalState.testcase.testEnabled) {
        std::string targetSize = "q";
        u8 checkpointID = readOperand(instruction.operands[0], targetSize, globalState);
        for (Testcases::Checkpoint& checkpoint : globalState.testcase.checkpoints) {
            if (checkpoint.id == checkpointID) {
                for (auto& [regName, value] : checkpoint.registers) {
                    bool isCorrect = false;
                    if (globalState.cpu.reg64.contains(regName)) {
                        isCorrect = *globalState.cpu.reg64[regName] == value;
                    }
                    else if (globalState.cpu.reg32.contains(regName)) {
                        isCorrect =  *globalState.cpu.reg32[regName] == static_cast<u32>(value);
                    }
                    else if (globalState.cpu.reg16.contains(regName)) {
                        isCorrect = *globalState.cpu.reg16[regName] == static_cast<u16>(value);
                    }
                    else if (globalState.cpu.reg8.contains(regName)) {
                        isCorrect = *globalState.cpu.reg8[regName] == static_cast<u8>(value);
                    }
                    if (!isCorrect) {
                        LOG_ERROR("Checkpoint {} failed: Register '{}' expected value '{:#x}', actual value '{:#x}'", checkpointID, regName, value,
                                  globalState.cpu.reg64.contains(regName) ? *globalState.cpu.reg64[regName] :
                                  globalState.cpu.reg32.contains(regName) ? *globalState.cpu.reg32[regName] :
                                  globalState.cpu.reg16.contains(regName) ? *globalState.cpu.reg16[regName] :
                                  *globalState.cpu.reg8[regName]);
                    }
                }
                for (auto& [flagName, flagValue] : checkpoint.flags) {
                    if (globalState.cpu.flags.contains(flagName)) {
                        if (*globalState.cpu.flags[flagName] != flagValue) {
                            LOG_ERROR("Checkpoint {} failed: Flag '{}' expected value '{}', actual value '{}'", checkpointID, flagName, flagValue,
                                      *globalState.cpu.flags[flagName]);
                        }
                    }
                }
                if (checkpoint.exit) {
                    LOG_INFO("Checkpoint {} requests program exit. Exiting.", checkpointID);
                    return 1;
                }
            }
        }
    }
    globalState.cpu.rip += 8;
    return 0;
}

} // namespace Interpreter::Instructions
