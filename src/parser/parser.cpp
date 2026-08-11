// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <vector>

#include "magic_enum_overrides.h"

#include "lexer/lexer.h"
#include "parser.h"
#include "interpreter/mnemonics.h"

#include "logging.h"

namespace Parser
{

bool isNumber(const std::string& str) {
    return !str.empty() && std::ranges::all_of(str, isdigit);
}

bool isNegativeNumber(const std::string& str) {
    return str.size() > 1 && str[0] == '-' && std::ranges::all_of(str.begin() + 1, str.end(), isdigit);
}

bool isHexNumber(const std::string& str) {
    return str.size() > 2 && str[0] == '0' && (str[1] == 'x') &&
           std::ranges::all_of(str.begin() + 2, str.end(), [](char c) {
               return std::isxdigit(std::tolower(static_cast<unsigned char>(c)));
           });
}

s64 textToNumber(const std::string& text) {
    std::string tmpText;
    for (char c : text) {
        tmpText.push_back(std::tolower(static_cast<unsigned char>(c)));
    }

    if (tmpText.find("0x") == 0) {
        return std::stoull(tmpText.substr(2), nullptr, 16);
    }

    return std::stoll(tmpText, nullptr, 10);
}

Ast::Register makeRegister(const Token& token) {
    const std::string name = token.lexeme.substr(1);
    auto it = registerTable.find(name);
    if (it == registerTable.end()) {
        LOG_ERROR("Unknown register '{}' (line {} column {})", name, token.line, token.column);
    }
    return it->second;
}

int parseOperand(Ast::Instruction& instruction, const std::vector<Token>& lineTokens, const u32 operandStart, const std::vector<u32>& operandCommaPositions) {
    u32 openBracketPosition = operandStart;
    bool hasDisplacement = false;
    if (lineTokens[operandStart].type != Token::Type::BracketOpen) {
        ++openBracketPosition;
        hasDisplacement = true;
    }

    switch (operandCommaPositions.size()) {
        case 0:
            // (base)
            instruction.operands.push_back(Ast::Memory{
                .base = makeRegister(lineTokens[openBracketPosition + 1])
            });

            break;

        case 1:
            // (base, index)
            instruction.operands.push_back(Ast::Memory{
                .base = makeRegister(lineTokens[openBracketPosition + 1]),
                .index = makeRegister(lineTokens[operandCommaPositions[0] + 1])
            });

            break;

        case 2:
            {
                // (base, index, scale)
                Ast::Scale scale;
                switch (std::stoull(lineTokens[operandCommaPositions[1] + 1].lexeme)) {
                    case 1:
                        scale = Ast::Scale::One;
                        break;

                    case 2:
                        scale = Ast::Scale::Two;
                        break;

                    case 4:
                        scale = Ast::Scale::Four;
                        break;

                    case 8:
                        scale = Ast::Scale::Eight;
                        break;

                    default:
                        LOG_ERROR("Invalid scale '{}' (line {} column {})", lineTokens[operandCommaPositions[1] + 1].lexeme, lineTokens[operandCommaPositions[1] + 1].line, lineTokens[operandCommaPositions[1] + 1].column);
                }
                instruction.operands.push_back(Ast::Memory{
                    .base = makeRegister(lineTokens[openBracketPosition + 1]),
                    .index = makeRegister(lineTokens[operandCommaPositions[0] + 1]),
                    .scale = scale
                });
                break;
            }
    }

    if (hasDisplacement) {
        auto dispToken = lineTokens[operandStart];
        if (dispToken.type == Token::Type::Number || dispToken.type == Token::Type::NegativeNumber) {
            std::get<Ast::Memory>(instruction.operands.back()).disp = textToNumber(dispToken.lexeme);
        }
        else if (dispToken.type == Token::Type::Identifier) {
            std::get<Ast::Memory>(instruction.operands.back()).disp = Ast::Label{ dispToken.lexeme };
        }
        else {
            LOG_ERROR("Not a valid displacement '{}' (line {} column {})", dispToken.lexeme, dispToken.line, dispToken.column);
        }
    }
    return 0;
}

int parseOperands(Ast::Instruction& instruction, const std::vector<Token>& lineTokens) {
    std::vector<std::vector<u32>> operandCommaPositions { {} , {} };
    bool inParen = false;
    u32 parameterCommaPos = 0;

    u32 operandIndex = 0;
    std::vector<u32> openBracketPositions { 0, 0 };
    for (u32 i = 1; i < lineTokens.size(); ++i) {
        switch (lineTokens[i].type) {
            case Token::Type::BracketOpen:
                openBracketPositions[operandIndex] = i;
                operandCommaPositions.push_back({});
                if (inParen) {
                    LOG_ERROR("Nested parentheses are not supported (line {} column {})", lineTokens[i].line, lineTokens[i].column);
                }
                inParen = true;
                break;

            case Token::Type::BracketClosed:
                if (!inParen) {
                    LOG_ERROR("Unmatched closing parenthesis (line {} column {})", lineTokens[i].line, lineTokens[i].column);
                }
                inParen = false;
                break;

            case Token::Type::Comma:
                if (inParen) {
                    if (operandCommaPositions[operandIndex].size() > 2) {
                        LOG_ERROR("More than three operands inside parentheses are not supported (line {} column {})", lineTokens[i].line, lineTokens[i].column);
                    }
                    operandCommaPositions[operandIndex].push_back(i);
                }
                else {
                    if (parameterCommaPos != 0) {
                        LOG_ERROR("More than two parameters are not supported (line {} column {})", lineTokens[i].line, lineTokens[i].column);
                    }
                    parameterCommaPos = i;
                    ++operandIndex;
                }
                break;
        }
    }

    if (parameterCommaPos != 0) {
        if (openBracketPositions[0] != 0) {
            parseOperand(instruction, lineTokens, 1, operandCommaPositions[0]); // Prefix like rep not supported yet
        }
        else {
            if (lineTokens[1].type == Token::Type::Register) {
                instruction.operands.push_back(makeRegister(lineTokens[1]));
            }
            else if (lineTokens[1].type == Token::Type::Immediate) {
                std::string immediateValue = lineTokens[1].lexeme.substr(1); // Remove '$'
                if (isNumber(immediateValue) || isHexNumber(immediateValue) || isNegativeNumber(immediateValue)) {
                    instruction.operands.push_back(Ast::Immediate{static_cast<u64>(textToNumber(immediateValue)) });
                }
                else if (lineTokens[2].type == Token::Type::Identifier) {
                    instruction.operands.push_back(Ast::Symbol{lineTokens[2].lexeme});
                }
            }
        }
        if (openBracketPositions[1] != 0) {
            parseOperand(instruction, lineTokens, parameterCommaPos + 1, operandCommaPositions[1]); // Same here
        }
        else {
            if (lineTokens[parameterCommaPos + 1].type == Token::Type::Register) {
                instruction.operands.push_back(makeRegister(lineTokens[parameterCommaPos + 1]));
            }
            else if (lineTokens[parameterCommaPos + 1].type == Token::Type::Immediate) {
                std::string immediateValue = lineTokens[parameterCommaPos + 1].lexeme.substr(1); // Remove '$'
                if (isNumber(immediateValue) || isHexNumber(immediateValue)) {
                    instruction.operands.push_back(Ast::Immediate{static_cast<u64>(textToNumber(immediateValue)) });
                }
            }
        }

    }
    else {
        if (lineTokens[1].type == Token::Type::Identifier) {
            instruction.operands.push_back(Ast::Symbol{lineTokens[1].lexeme});
        }
        else if (lineTokens[1].type == Token::Type::Register) {
            instruction.operands.push_back(makeRegister(lineTokens[1]));
        }
        else if (lineTokens[1].type == Token::Type::Immediate) {
            std::string immediateValue = lineTokens[1].lexeme.substr(1); // Remove '$'
            if (isNumber(immediateValue) || isHexNumber(immediateValue)) {
                instruction.operands.push_back(Ast::Immediate{static_cast<u64>(textToNumber(immediateValue)) });
            }
        }
    }
    return 0;
}

bool operandMatches(const Interpreter::Mnemonics::OperandSpec& spec, const Ast::Operand& operand) {
    using OpType = Interpreter::Mnemonics::OpType;
    switch (spec.type) {
        case OpType::Register:
            return std::holds_alternative<Ast::Register>(operand);

        case OpType::Memory:
        case OpType::MemoryNoSize:
            return std::holds_alternative<Ast::Memory>(operand);

        case OpType::RegisterOrMemory:
            return std::holds_alternative<Ast::Register>(operand)
                || std::holds_alternative<Ast::Memory>(operand);

        case OpType::Immediate:
            return std::holds_alternative<Ast::Immediate>(operand)
                || std::holds_alternative<Ast::Symbol>(operand);

        case OpType::Relative:
            return std::holds_alternative<Ast::RelativeImmediate>(operand)
                || std::holds_alternative<Ast::Symbol>(operand);
    }
    return false;
}

bool formMatches(const Interpreter::Mnemonics::InstructionForm& form, const std::vector<Ast::Operand>& operands) {
    if (form.size() != operands.size()) {
        return false;
    }
    for (u32 i = 0; i < operands.size(); ++i) {
        if (!operandMatches(form[i], operands[i])) {
            return false;
        }
    }
    return true;
}

const std::vector<Interpreter::Mnemonics::OperandSpec>* findMatchingForm(
    const Interpreter::Mnemonics::InstructionDetails& instructionDef, const std::vector<Ast::Operand>& operands) {
    for (const auto& form : instructionDef.forms) {
        if (formMatches(form, operands)) {
            return &form;
        }
    }
    return nullptr;
}

void resolveRelativeOperands(const Interpreter::Mnemonics::InstructionForm& form, std::vector<Ast::Operand>& operands) {
    for (u32 i = 0; i < operands.size(); ++i) {
        if (form[i].type != Interpreter::Mnemonics::OpType::Relative) {
            continue;
        }
        if (std::holds_alternative<Ast::Symbol>(operands[i])) {
            operands[i] = Ast::RelativeImmediate{
                Ast::Label{ std::get<Ast::Symbol>(operands[i]).name }
            };
        }
    }
}
int parse(const std::vector<Token>& tokens, std::vector<Ast::Section>& ast) {
    std::vector<std::vector<Token>> inputLines { std::vector<Token>{} };
    u32 lineNumber = 0;
    for (Token token : tokens) {
        inputLines[lineNumber].push_back(token);

        if (token.type == Token::Type::EOL) {
            ++lineNumber;
            inputLines.push_back(std::vector<Token>{});
        }
    }

    for (const auto& lineTokens : inputLines) {
        if (lineTokens.empty()) {
            continue;
        }

        if (lineTokens[0].type == Token::Type::Dot) {
            if (lineTokens[1].type == Token::Type::Identifier) {
                if (lineTokens[1].lexeme == "section") {
                    if (lineTokens[2].type == Token::Type::Identifier) {
                        Ast::Section section = {lineTokens[2].lexeme.substr(1), {} };
                        ast.push_back(section);
                    }
                }
                else if (lineTokens[1].lexeme == "text" || lineTokens[1].lexeme == "data" || lineTokens[1].lexeme == "bss" || lineTokens[1].lexeme == "rodata") {
                    Ast::Section section = {lineTokens [1].lexeme, {} };
                    ast.push_back(section);
                }
                else if (auto value = magic_enum::enum_cast<Ast::Directive::Name>(lineTokens[1].lexeme)) {
                    Ast::Directive directive;
                    directive.name = *value;
                    for (u32 i = 2; i < lineTokens.size() - 1; ++i) { // Exclude EOL
                        if (lineTokens[i].type == Token::Type::Comma) {
                            continue;
                        }
                        directive.arguments.push_back(lineTokens[i].lexeme);
                    }
                    if (ast.empty()) {
                        LOG_INFO("Implicit .text section created");
                        ast.push_back(Ast::Section{ "text", {} });
                    }
                    ast.back().items.push_back(directive);
                }
                else if (auto ignored = magic_enum::enum_cast<IgnoredDirectives>(lineTokens[1].lexeme)) {
                    LOG_WARNING("Ignoring directive '{}' at line {} column {}", lineTokens[1].lexeme, lineTokens[1].line, lineTokens[1].column);
                }
                else if (lineTokens.size() == 4 && lineTokens[1].type == Token::Type::Identifier && lineTokens[2].type == Token::Type::Colon) {
                    ast.back().items.push_back(Ast::Label{ "." + lineTokens[1].lexeme });
                }
                else {
                    LOG_WARNING("Unknown directive '{}' at line {} column {}", lineTokens[1].lexeme, lineTokens[1].line, lineTokens[1].column);
                }
            }
        }

        if (lineTokens[0].type == Token::Type::Identifier) {
            std::string mnemonicName {};
            std::string prefix {};
            std::string suffix {};
            u8 mnemonicPos = 0;

            auto possiblePrefixes = Interpreter::Mnemonics::populatePossiblePrefixes();
            if (std::ranges::find(possiblePrefixes, lineTokens[0].lexeme) != possiblePrefixes.end()) {
                prefix = lineTokens[0].lexeme;
                mnemonicPos = 1;
            }
            if (Interpreter::Mnemonics::instructionDefinitions.contains(lineTokens[mnemonicPos].lexeme)) {
                mnemonicName = lineTokens[mnemonicPos].lexeme;
            }
            else if (Interpreter::Mnemonics::instructionDefinitions.contains(lineTokens[mnemonicPos].lexeme.substr(0, lineTokens[mnemonicPos].lexeme.size() - 1))) {
                mnemonicName = lineTokens[mnemonicPos].lexeme.substr(0, lineTokens[mnemonicPos].lexeme.size() - 1);
                suffix = lineTokens[0].lexeme.substr(lineTokens[0].lexeme.size() - 1);
            }

            if (!mnemonicName.empty()) {
                // Instruction found
                Ast::Instruction instruction;
                Ast::Mnemonic mnemonic;
                mnemonic.mnemonicName = mnemonicName;

                auto& instructionDef = Interpreter::Mnemonics::instructionDefinitions[mnemonicName];
                if (!prefix.empty()) {
                    if (std::ranges::find(instructionDef.allowedPrefixes, prefix) == instructionDef.allowedPrefixes.end()) {
                        LOG_ERROR("Invalid prefix '{}' for mnemonic '{}' at line {} column {}", prefix, mnemonicName, lineTokens[0].line, lineTokens[0].column);
                    }
                    mnemonic.prefix = prefix;
                }
                if (!suffix.empty()) {
                    if (std::ranges::find(instructionDef.allowedSuffixes, suffix) == instructionDef.allowedSuffixes.end()) {
                        LOG_ERROR("Invalid suffix '{}' for mnemonic '{}' at line {} column {}", suffix, mnemonicName, lineTokens[0].line, lineTokens[0].column);
                    }
                    mnemonic.width = suffixWidths.at(suffix);
                }
                instruction.mnemonic = mnemonic;
                parseOperands(instruction, lineTokens);

                const auto* form = findMatchingForm(instructionDef, instruction.operands);
                if (form == nullptr) {
                    LOG_ERROR("Invalid operands for mnemonic '{}' at line {} column {}", mnemonicName, lineTokens[0].line, lineTokens[0].column);
                }
                resolveRelativeOperands(*form, instruction.operands);
                ast.back().items.push_back(instruction);

                continue;
            }

            else {
                std::optional<Ast::CondCode> condCode;
                std::string mnemonicName;

                std::string tmp = lineTokens[0].lexeme.substr(1);
                if (condCodeMap.contains(tmp)) {
                    condCode = condCodeMap[tmp];
                    mnemonicName = "Jcc";
                }
                if (lineTokens[0].lexeme.size() >= 5 && lineTokens[0].lexeme.starts_with("cmov")) {
                    tmp = lineTokens[0].lexeme.substr(4, 2);
                    if (condCodeMap.contains(tmp)) {
                        condCode = condCodeMap[tmp];
                        mnemonicName = "CMOVcc";
                    }
                    if (lineTokens[0].lexeme.size() == 7) {
                        suffix = lineTokens[0].lexeme.substr(6);
                        auto& instructionDef = Interpreter::Mnemonics::instructionDefinitions[mnemonicName];
                        if (std::ranges::find(instructionDef.allowedSuffixes, suffix) == instructionDef.allowedSuffixes.end()) {
                            LOG_ERROR("Invalid suffix '{}' for mnemonic '{}' at line {} column {}", suffix, mnemonicName, lineTokens[0].line, lineTokens[0].column);
                        }
                    }
                }
                if (condCode.has_value()) {
                    Ast::Instruction instruction;
                    Ast::Mnemonic mnemonic;
                    mnemonic.mnemonicName = mnemonicName;
                    if (!suffix.empty()) {
                        mnemonic.width = suffixWidths.at(suffix);
                    }

                    instruction.mnemonic = mnemonic;
                    instruction.additionalData = condCode;
                    parseOperands(instruction, lineTokens);

                    auto& instructionDef = Interpreter::Mnemonics::instructionDefinitions[mnemonicName];
                    const auto* form = findMatchingForm(instructionDef, instruction.operands);
                    if (form == nullptr) {
                        LOG_ERROR("Invalid operands for mnemonic '{}' at line {} column {}", mnemonicName, lineTokens[0].line, lineTokens[0].column);
                    }
                    resolveRelativeOperands(*form, instruction.operands);

                    ast.back().items.push_back(instruction);
                    continue;
                }
            }

            // Labels
            if (lineTokens[0].type == Token::Type::Identifier && lineTokens[1].type == Token::Type::Colon && lineTokens[2].type == Token::Type::EOL) {
                if (ast.empty()) {
                    LOG_INFO("Implicit .text section created");
                    ast.push_back(Ast::Section{ "text", {} });
                }
                ast.back().items.push_back(Ast::Label{ lineTokens[0].lexeme });
            }
            // Symbol assignments
            else if (lineTokens[0].type == Token::Type::Identifier && lineTokens[1].type == Token::Type::Equal) {
                ast.back().items.push_back(Ast::SymbolAssignment{
                    .name = lineTokens[0].lexeme,
                    .expression = Ast::Expression{ std::vector<Token>{ lineTokens.begin() + 2, lineTokens.end() - 1 } } // Exclude EOL
                    });
            }
            else {
                LOG_WARNING("Unknown mnemonic '{}' at line {} column {}", lineTokens[0].lexeme, lineTokens[0].line, lineTokens[0].column);
            }
        }
    }

    return 0;

}

} // namespace Parser