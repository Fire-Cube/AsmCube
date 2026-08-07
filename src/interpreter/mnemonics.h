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
        Register,
        Memory,
        RegisterOrMemory,
        Immediate,
        Relative,
        MemoryNoSize,
    };
    Type type;
    std::vector<u8> sizes;
};

using InstructionForm = std::vector<OperandSpec>;
using OpType = OperandSpec::Type;

struct InstructionDetails {
    InstructionSet instructionSet;
    std::vector<std::string> allowedPrefixes;
    std::vector<std::string> allowedSuffixes;
    std::vector<InstructionForm> forms;
    u32 (*implementation)(GlobalState&, Ast::Instruction&);
};

inline std::vector<std::string> integerSizeSuffixes = {
    "b", // byte
    "w", // word
    "l", // long
    "q", // quad
};

inline std::vector<u8> All { 8, 16, 32, 64 };
inline std::vector<u8> WordAndUp { 16, 32, 64 };

inline std::vector<InstructionForm> NormalForms {
    {{ OpType::Register, All }, { OpType::RegisterOrMemory, All }},
    {{ OpType::Memory, All }, { OpType::Register, All }},
    {{ OpType::Immediate, All }, { OpType::RegisterOrMemory, All }},
};

inline std::vector<InstructionForm> NoMemoryForms {
    {{ OpType::Register, All }, { OpType::RegisterOrMemory, All }},
    {{ OpType::Immediate, All }, { OpType::RegisterOrMemory, All }},
};

inline std::vector<InstructionForm> SingleOpOnlyRMForms {
    {{ OpType::RegisterOrMemory, All }},
};

inline std::vector<InstructionForm> NoOperandsForms {
    {}
};

inline std::unordered_map<std::string, InstructionDetails> instructionDefinitions = {
    {"lea", {InstructionSet::x86_64, {}, integerSizeSuffixes, {
        {{ OpType::MemoryNoSize, {} }, { OpType::Register, WordAndUp }},
    }, Instructions::lea }},
    {"mov", {InstructionSet::x86_64, {}, integerSizeSuffixes, NormalForms, Instructions::mov }},
    {"xor", {InstructionSet::x86_64, {}, integerSizeSuffixes, NormalForms, Instructions::Xor }},
    {"and", {InstructionSet::x86_64, {}, integerSizeSuffixes, NormalForms, Instructions::And }},
    {"add", {InstructionSet::x86_64, {}, integerSizeSuffixes, NormalForms, Instructions::add }},
    {"sub", {InstructionSet::x86_64, {}, integerSizeSuffixes, NormalForms, Instructions::sub }},
    {"cmp", {InstructionSet::x86_64, {}, integerSizeSuffixes, NormalForms, Instructions::cmp }},
    {"inc", {InstructionSet::x86_64, {}, integerSizeSuffixes, SingleOpOnlyRMForms, Instructions::inc }},
    {"dec", {InstructionSet::x86_64, {}, integerSizeSuffixes, SingleOpOnlyRMForms, Instructions::dec }},
    {"neg", {InstructionSet::x86_64, {}, integerSizeSuffixes, SingleOpOnlyRMForms, Instructions::neg }},
    {"test", {InstructionSet::x86_64, {}, integerSizeSuffixes, NoMemoryForms, Instructions::test }},
    {"push", {InstructionSet::x86_64, {}, {"w", "q"}, {
        {{ OpType::RegisterOrMemory, {16, 64} }},
        {{ OpType::Immediate, {8, 16, 32} }},
    }, Instructions::push }},
    {"pop", {InstructionSet::x86_64, {}, {"w", "q"}, {
            {{ OpType::RegisterOrMemory, {16, 64} }},
    }, Instructions::pop }},
    {"call", {InstructionSet::x86_64, {}, {"q"}, {
        {{ OpType::Relative, {32} }},
            {{ OpType::RegisterOrMemory, {64} }},
    }, Instructions::call }},
    {"ret", {InstructionSet::x86_64, {}, {"q"}, {
        {},
        {{ OpType::Immediate, {16} }},
    }, Instructions::ret }},
    {"jmp", {InstructionSet::x86_64, {}, {"q"}, {
        {{ OpType::Relative, {8, 32} }},
        {{ OpType::RegisterOrMemory, {64} }},
    }, Instructions::jmp }},
    {"Jcc", {InstructionSet::x86_64, {}, {"q"}, {
        {{ OpType::Relative, {8, 32} }},
    }, Instructions::Jcc }},
    {"CMOVcc", {InstructionSet::x86_64, {}, integerSizeSuffixes, {
        {{ OpType::RegisterOrMemory, WordAndUp }, { OpType::Register, WordAndUp }},
    }, Instructions::CMOVcc }},
    {"stc", {InstructionSet::x86_64, {}, {}, NoOperandsForms, Instructions::stc }},
    {"hlt", {InstructionSet::x86_64, {}, {}, NoOperandsForms, Instructions::hlt }},
    {"leave", {InstructionSet::x86_64, {}, {}, NoOperandsForms, Instructions::leave }},
    {"syscall", {InstructionSet::x86_64, {}, {}, NoOperandsForms, Instructions::syscall }},
    {"checkpoint", {InstructionSet::custom, {}, {}, {
        {{ OpType::Immediate, {64} }},
    }, Instructions::checkpoint }},
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