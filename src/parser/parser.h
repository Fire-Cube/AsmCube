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

bool isNumber(const std::string& text);
bool isHexNumber(const std::string& text);
s64 textToNumber(const std::string& text);
int parseOperand(const Ast::Instruction& instruction, const std::vector<Token>& lineTokens, u32 operandStart, const std::vector<u32>& operandCommaPositions);
int parse(const std::vector<Token>& tokens, std::vector<Ast::Section>& ast);

} // namespace Parser