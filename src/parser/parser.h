// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

#include "types.h"
#include "lexer/lexer.h"

#include "ast.h"

namespace Parser
{

enum class IgnoredDirectives {
    type,
    cfi_startproc,
    cfi_endproc,
    cfi_undefined,
    size,
    file,
};

inline const std::unordered_map<std::string, Ast::Width> suffixWidths = {
    {"b", Ast::Width::Byte},
    {"w", Ast::Width::Word},
    {"l", Ast::Width::Long},
    {"q", Ast::Width::Quad},
};

inline std::unordered_map<std::string, Ast::CondCode> condCodeMap = {
    {"o", Ast::CondCode::overflow},
    {"no", Ast::CondCode::notOverflow},
    {"s", Ast::CondCode::sign},
    {"ns", Ast::CondCode::notSign},
    {"e", Ast::CondCode::equal},
    {"z", Ast::CondCode::zero},
    {"ne", Ast::CondCode::notEqual},
    {"nz", Ast::CondCode::notZero},
    {"b", Ast::CondCode::below},
    {"nae", Ast::CondCode::notAboveOrEqual},
    {"c", Ast::CondCode::carry},
    {"nb", Ast::CondCode::notBelow},
    {"ae", Ast::CondCode::aboveOrEqual},
    {"nc", Ast::CondCode::notCarry},
    {"be", Ast::CondCode::belowOrEqual},
    {"na", Ast::CondCode::notAbove},
    {"a", Ast::CondCode::above},
    {"nbe", Ast::CondCode::notBelowOrEqual},
    {"l", Ast::CondCode::less},
    {"nge", Ast::CondCode::notGreaterOrEqual},
    {"ge", Ast::CondCode::greaterOrEqual},
    {"nl", Ast::CondCode::notLess},
    {"le", Ast::CondCode::lessOrEqual},
    {"ng", Ast::CondCode::notGreater},
    {"g", Ast::CondCode::greater},
    {"nle", Ast::CondCode::notLessOrEqual},
    {"p", Ast::CondCode::parity},
    {"pe", Ast::CondCode::parityEven},
    {"np", Ast::CondCode::notParity},
    {"po", Ast::CondCode::parityOdd},
};

inline std::unordered_map<std::string, Ast::Register> registerTable = {
    { "rax", { "rax", Ast::Width::Quad, 0 }},
    { "rbx", { "rbx", Ast::Width::Quad, 1 }},
    { "rcx", { "rcx", Ast::Width::Quad, 2 }},
    { "rdx", { "rdx", Ast::Width::Quad, 3 }},
    { "rsi", { "rsi", Ast::Width::Quad, 4 }},
    { "rdi", { "rdi", Ast::Width::Quad, 5 }},
    { "rsp", { "rsp", Ast::Width::Quad, 6 }},
    { "rbp", { "rbp", Ast::Width::Quad, 7 }},
    { "r8", { "r8", Ast::Width::Quad, 8 }},
    { "r9", { "r9", Ast::Width::Quad, 9 }},
    { "r10", { "r10", Ast::Width::Quad, 10 }},
    { "r11", { "r11", Ast::Width::Quad, 11 }},
    { "r12", { "r12", Ast::Width::Quad, 12 }},
    { "r13", { "r13", Ast::Width::Quad, 13 }},
    { "r14", { "r14", Ast::Width::Quad, 14 }},
    { "r15", { "r15", Ast::Width::Quad, 15 }},
    { "rip", { "rip", Ast::Width::Quad, 16 }},
    { "eax", { "eax", Ast::Width::Long, 0 }},
    { "ebx", { "ebx", Ast::Width::Long, 1 }},
    { "ecx", { "ecx", Ast::Width::Long, 2 }},
    { "edx", { "edx", Ast::Width::Long, 3 }},
    { "esi", { "esi", Ast::Width::Long, 4 }},
    { "edi", { "edi", Ast::Width::Long, 5 }},
    { "esp", { "esp", Ast::Width::Long, 6 }},
    { "ebp", { "ebp", Ast::Width::Long, 7 }},
    { "r8d", { "r8d", Ast::Width::Long, 8 }},
    { "r9d", { "r9d", Ast::Width::Long, 9 }},
    { "r10d", { "r10d", Ast::Width::Long, 10 }},
    { "r11d", { "r11d", Ast::Width::Long, 11 }},
    { "r12d", { "r12d", Ast::Width::Long, 12 }},
    { "r13d", { "r13d", Ast::Width::Long, 13 }},
    { "r14d", { "r14d", Ast::Width::Long, 14 }},
    { "r15d", { "r15d", Ast::Width::Long, 15 }},
    { "eip", { "eip", Ast::Width::Long, 16 }},
    { "ax", { "ax", Ast::Width::Word, 0 }},
    { "bx", { "bx", Ast::Width::Word, 1 }},
    { "cx", { "cx", Ast::Width::Word, 2 }},
    { "dx", { "dx", Ast::Width::Word, 3 }},
    { "si", { "si", Ast::Width::Word, 4 }},
    { "di", { "di", Ast::Width::Word, 5 }},
    { "sp", { "sp", Ast::Width::Word, 6 }},
    { "bp", { "bp", Ast::Width::Word, 7 }},
    { "r8w", { "r8w", Ast::Width::Word, 8 }},
    { "r9w", { "r9w", Ast::Width::Word, 9 }},
    { "r10w", { "r10w", Ast::Width::Word, 10 }},
    { "r11w", { "r11w", Ast::Width::Word, 11 }},
    { "r12w", { "r12w", Ast::Width::Word, 12 }},
    { "r13w", { "r13w", Ast::Width::Word, 13 }},
    { "r14w", { "r14w", Ast::Width::Word, 14 }},
    { "r15w", { "r15w", Ast::Width::Word, 15 }},
    { "ip", { "ip", Ast::Width::Word, 16 }},
    { "ah", { "ah", Ast::Width::Byte, 0 }},
    { "bh", { "bh", Ast::Width::Byte, 1 }},
    { "ch", { "ch", Ast::Width::Byte, 2 }},
    { "dh", { "dh", Ast::Width::Byte, 3 }},
    { "al", { "al", Ast::Width::Byte, 4 }},
    { "bl", { "bl", Ast::Width::Byte, 5 }},
    { "cl", { "cl", Ast::Width::Byte, 6 }},
    { "dl", { "dl", Ast::Width::Byte, 7 }},
    { "sil", { "sil", Ast::Width::Byte, 8 }},
    { "dil", { "dil", Ast::Width::Byte, 9 }},
    { "spl", { "spl", Ast::Width::Byte, 10 }},
    { "bpl", { "bpl", Ast::Width::Byte, 11 }},
    { "r8b", { "r8b", Ast::Width::Byte, 12 }},
    { "r9b", { "r9b", Ast::Width::Byte, 13 }},
    { "r10b", { "r10b", Ast::Width::Byte, 14 }},
    { "r11b", { "r11b", Ast::Width::Byte, 15 }},
    { "r12b", { "r12b", Ast::Width::Byte, 16 }},
    { "r13b", { "r13b", Ast::Width::Byte, 17 }},
    { "r14b", { "r14b", Ast::Width::Byte, 18 }},
    { "r15b", { "r15b", Ast::Width::Byte, 19 }}
};

bool isNumber(const std::string& text);
bool isHexNumber(const std::string& text);
s64 textToNumber(const std::string& text);
int parseOperand(const Ast::Instruction& instruction, const std::vector<Token>& lineTokens, u32 operandStart, const std::vector<u32>& operandCommaPositions);
int parse(const std::vector<Token>& tokens, std::vector<Ast::Section>& ast);

} // namespace Parser