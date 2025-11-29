#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "interpreter.h"
#include "registers.h"
#include "memory.h"
#include "types.h"
#include "instructions.h"

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
};

struct OperandSpec {
    enum class Type {
        Immediate,
        Register,
        Memory,
    };
    Type operand1;
    Type operand2;
};

struct InstructionDetails {
    InstructionSet instructionSet;
    std::vector<u64> allowedSizes { 8, 16, 32, 64 };
    std::vector<std::string> allowedPrefixes;
    std::vector<std::string> allowedSuffixes;
    std::vector<OperandSpec> operandSpecs;
    std::function<u32(GlobalState&, Instruction&)> implementation;
};

inline std::vector<std::string> integerSizeSuffixes = {
    "b", // byte
    "w", // word
    "l", // long
    "q", // quad
};

inline std::vector<OperandSpec> NormalOperands = std::vector<OperandSpec> {
    { OperandSpec::Type::Register, OperandSpec::Type::Register },
    { OperandSpec::Type::Register, OperandSpec::Type::Memory },
    { OperandSpec::Type::Memory,   OperandSpec::Type::Register },
    { OperandSpec::Type::Memory, OperandSpec::Type::Memory },
    { OperandSpec::Type::Immediate, OperandSpec::Type::Register },
    { OperandSpec::Type::Immediate, OperandSpec::Type::Memory },
};

inline std::unordered_map<std::string, InstructionDetails> instructionDefinitions = {
    {"lea", {InstructionSet::x86_64, {64}, {}, integerSizeSuffixes, {
        {
            OperandSpec::Type::Memory, OperandSpec::Type::Register
        }
        }, Instructions::lea}
    },
    {"mov", {InstructionSet::x86_64, {8, 16, 32, 64}, {}, integerSizeSuffixes, {}, Instructions::mov}},
    {"xor", {InstructionSet::x86_64, {8, 16, 32, 64}, {}, {}, {}, Instructions::Xor}},
    {"syscall", {InstructionSet::x86_64, {}, {}, {}, {},Instructions::syscall}}
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
