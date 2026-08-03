// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "types.h"
#include "instructions.h"

namespace Interpreter::Mnemonics
{

enum class InstructionSet {
    x86_64,
    AES,
    AVX,
    AVX2,
    AVX512,
    SSE,
    SSE2,
    SSE3,
    SSSE3,
    SSE4_1,
    SSE4_2,
    SSE_4A,
    custom
};

struct OperandSpec {
    enum class Type {
        Immediate,
        Register,
        Memory,
    };
    Type operand1;
    std::optional<Type> operand2;
};

struct InstructionDetails {
    InstructionSet instructionSet;
    std::vector<u64> allowedSizes;
    std::vector<std::string> allowedPrefixes;
    std::vector<std::string> allowedSuffixes;
    std::vector<OperandSpec> operandSpecs;
    std::function<u32(GlobalState&, Ast::Instruction&)> implementation;
};

inline std::vector<std::string> integerSizeSuffixes = {
    "b", // byte
    "w", // word
    "l", // long
    "q", // quad
};

inline std::vector<OperandSpec> NormalOperands = std::vector<OperandSpec> {
    { OperandSpec::Type::Register,  OperandSpec::Type::Register },
    { OperandSpec::Type::Register,  OperandSpec::Type::Memory },
    { OperandSpec::Type::Memory,    OperandSpec::Type::Register },
    { OperandSpec::Type::Immediate, OperandSpec::Type::Register },
    { OperandSpec::Type::Immediate, OperandSpec::Type::Memory },
};

inline std::vector<OperandSpec> NormalOperandsNoMemorySource = std::vector<OperandSpec> {
    { OperandSpec::Type::Register,  OperandSpec::Type::Register },
    { OperandSpec::Type::Register,  OperandSpec::Type::Memory },
    { OperandSpec::Type::Immediate, OperandSpec::Type::Register },
    { OperandSpec::Type::Immediate, OperandSpec::Type::Memory },
};

inline std::vector<OperandSpec> SingleOperands = std::vector<OperandSpec> {
    { OperandSpec::Type::Register },
    { OperandSpec::Type::Memory },
    { OperandSpec::Type::Immediate },
};

inline std::vector<OperandSpec> SingleOperandsNoImmediate = std::vector<OperandSpec> {
    { OperandSpec::Type::Register },
    { OperandSpec::Type::Memory },
};

inline std::vector<OperandSpec> MemoryRegisterOperand = std::vector<OperandSpec> {
    { OperandSpec::Type::Memory, OperandSpec::Type::Register },
};

inline std::unordered_map<std::string, InstructionDetails> instructionDefinitions = {
    {"lea", {InstructionSet::x86_64, {16, 32, 64}, {}, integerSizeSuffixes, MemoryRegisterOperand, Instructions::lea }},
    {"mov", {InstructionSet::x86_64, {8, 16, 32, 64}, {}, integerSizeSuffixes, NormalOperands, Instructions::mov }},
    {"xor", {InstructionSet::x86_64, {8, 16, 32, 64}, {}, integerSizeSuffixes, NormalOperands, Instructions::Xor }},
    {"and", {InstructionSet::x86_64, {8, 16, 32, 64}, {}, integerSizeSuffixes, NormalOperands, Instructions::And }},
    {"add", {InstructionSet::x86_64, {8, 16, 32, 64}, {}, integerSizeSuffixes, NormalOperands, Instructions::add }},
    {"sub", {InstructionSet::x86_64, {8, 16, 32, 64}, {}, integerSizeSuffixes, NormalOperands, Instructions::sub }},
    {"cmp", {InstructionSet::x86_64, {8, 16, 32, 64}, {}, integerSizeSuffixes, NormalOperands, Instructions::cmp }},
    {"inc", {InstructionSet::x86_64, {8, 16, 32, 64}, {}, integerSizeSuffixes, SingleOperands, Instructions::inc }},
    {"dec", {InstructionSet::x86_64, {8, 16, 32, 64}, {}, integerSizeSuffixes, SingleOperands, Instructions::dec }},
    {"neg", {InstructionSet::x86_64, {8, 16, 32, 64}, {}, integerSizeSuffixes, SingleOperands, Instructions::neg }},
    {"test", {InstructionSet::x86_64, {8, 16, 32, 64}, {}, integerSizeSuffixes, NormalOperandsNoMemorySource, Instructions::test }},
    {"stc", {InstructionSet::x86_64, {}, {}, {}, {}, Instructions::stc }},
    {"push", {InstructionSet::x86_64, {8, 16, 32, 64}, {}, integerSizeSuffixes, SingleOperands, Instructions::push }},
    {"pop", {InstructionSet::x86_64, {8, 16, 32, 64}, {}, integerSizeSuffixes, SingleOperandsNoImmediate, Instructions::pop }},
    {"call", {InstructionSet::x86_64, {64}, {}, {"q"}, {{OperandSpec::Type::Immediate}},Instructions::call }},
    {"ret", {InstructionSet::x86_64, {}, {}, {"q"}, {}, Instructions::ret }},
    {"jmp", {InstructionSet::x86_64, {64}, {}, {"q"}, {{OperandSpec::Type::Immediate}}, Instructions::jmp }},
    {"Jcc", {InstructionSet::x86_64, {64}, {}, {"q"}, {{OperandSpec::Type::Immediate}}, Instructions::Jcc }},
    {"CMOVcc", {InstructionSet::x86_64, {8, 16, 32, 64}, {}, integerSizeSuffixes, NormalOperands, Instructions::CMOVcc }},
    {"hlt", {InstructionSet::x86_64, {}, {}, {}, {}, Instructions::hlt }},
    {"leave", {InstructionSet::x86_64, {}, {}, {}, {}, Instructions::leave }},
    {"syscall", {InstructionSet::x86_64, {}, {}, {}, {}, Instructions::syscall }},
    {"checkpoint", {InstructionSet::custom, {64}, {}, {}, {{OperandSpec::Type::Immediate}}, Instructions::checkpoint }},
};

inline std::vector<std::string> populatePossiblePrefixes() {
    std::vector<std::string> possiblePrefixes = {};
    for (const auto& [mnemonicName, details] : instructionDefinitions) {
        for (const auto& prefix : details.allowedPrefixes) {
            if (std::ranges::find(possiblePrefixes, prefix) == possiblePrefixes.end()) {
                possiblePrefixes.push_back(prefix);
            }
        }
    }
    return possiblePrefixes;
}

} // namespace Interpreter::Mnemonics