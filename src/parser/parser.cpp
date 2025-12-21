// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <vector>

#include "magic_enum_overrides.h"

#include "lexer/lexer.h"
#include "parser.h"
#include "interpreter/mnemonics.h"

#include "logging.h"

bool isNumber(const std::string& str) {
    return !str.empty() && std::ranges::all_of(str, isdigit);
}

int parseOperand(Instruction& instruction, const std::vector<Token>& lineTokens, const u32 operandStart, const std::vector<u32>& operandCommaPositions) {
    u32 openBracketPosition = operandStart;
    bool hasDisplacement = false;
    if (lineTokens[operandStart].type != Token::Type::BracketOpen) {
        ++openBracketPosition;
        hasDisplacement = true;
    }

    switch (operandCommaPositions.size()) {
        case 0:
            // (base)
            instruction.operands.push_back(Memory{
                .base = Register{ lineTokens[openBracketPosition + 1].lexeme.substr(1) }
            });

            break;

        case 1:
            // (base, index)
            instruction.operands.push_back(Memory{
                .base = Register{ lineTokens[openBracketPosition + 1].lexeme.substr(1) },
                .index = Register{ lineTokens[operandCommaPositions[0] + 1].lexeme.substr(1) }
            });

            break;

        case 2:
            {
                // (base, index, scale)
                Scale scale;
                switch (std::stoull(lineTokens[operandCommaPositions[1] + 1].lexeme)) {
                    case 1:
                        scale = Scale::One;
                        break;

                    case 2:
                        scale = Scale::Two;
                        break;

                    case 4:
                        scale = Scale::Four;
                        break;

                    case 8:
                        scale = Scale::Eight;
                        break;

                    default:
                        LOG_ERROR("Invalid scale '{}' (line {} column {})", lineTokens[operandCommaPositions[1] + 1].lexeme, lineTokens[operandCommaPositions[1] + 1].line, lineTokens[operandCommaPositions[1] + 1].column);
                }
                instruction.operands.push_back(Memory{
                    .base = Register{ lineTokens[openBracketPosition + 1].lexeme.substr(1) },
                    .index = Register{ lineTokens[operandCommaPositions[0] + 1].lexeme.substr(1) },
                    .scale = scale
                });
                break;
            }
    }

    if (hasDisplacement) {
        auto dispToken = lineTokens[operandStart];
        if (dispToken.type == Token::Type::Number || dispToken.type == Token::Type::NegativeNumber) {
            std::get<Memory>(instruction.operands.back()).disp = std::stoll(dispToken.lexeme);
        }
        else if (dispToken.type == Token::Type::Identifier) {
            std::get<Memory>(instruction.operands.back()).disp = Label{ dispToken.lexeme };
        }
        else {
            LOG_ERROR("Not a valid displacement '{}' (line {} column {})", dispToken.lexeme, dispToken.line, dispToken.column);
        }
    }
    return 0;
}

int parseOperands(Instruction& instruction, const std::vector<Token>& lineTokens, std::vector<Section>& ast) {
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
                instruction.operands.push_back(Register{ lineTokens[1].lexeme });
            }
            else if (lineTokens[1].type == Token::Type::Immediate) {
                std::string immediateValue = lineTokens[1].lexeme.substr(1); // Remove '$'
                if (isNumber(immediateValue)) {
                    instruction.operands.push_back(Immediate{ .integer = std::stoull(immediateValue) });
                }
                else if (lineTokens[2].type == Token::Type::Identifier) {
                    instruction.operands.push_back(Immediate{ .symbol = lineTokens[2].lexeme});
                }
            }
        }
        if (openBracketPositions[1] != 0) {
            parseOperand(instruction, lineTokens, parameterCommaPos + 1, operandCommaPositions[1]); // Same here
        }
        else {
            if (lineTokens[parameterCommaPos + 1].type == Token::Type::Register) {
                instruction.operands.push_back(Register{ lineTokens[parameterCommaPos + 1].lexeme });
            }
        }

    }
    else {
        if (lineTokens[1].type == Token::Type::Identifier) {
            instruction.operands.push_back(Immediate{ .symbol = lineTokens[1].lexeme});
        }
        else if (lineTokens[1].type == Token::Type::Register) {
            instruction.operands.push_back(Register{ lineTokens[1].lexeme });
        }
    }
    ast.back().items.push_back(instruction);
    return 0;
}

int parse(const std::vector<Token>& tokens, std::vector<Section>& ast) {
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
                    if (lineTokens[2].type == Token::Type::Dot && lineTokens[3].type == Token::Type::Identifier) {
                        Section section = {lineTokens[3].lexeme, {} };
                        ast.push_back(section);
                    }
                }
                else if (lineTokens[1].lexeme == "text" || lineTokens[1].lexeme == "data" || lineTokens[1].lexeme == "bss" || lineTokens[1].lexeme == "rodata") {
                    Section section = {lineTokens [1].lexeme, {} };
                    ast.push_back(section);
                }
                else if (auto value = magic_enum::enum_cast<Directive::Name>(lineTokens[1].lexeme)) {
                    Directive directive;
                    directive.name = *value;
                    for (u32 i = 2; i < lineTokens.size() - 1; ++i) { // Exclude EOL
                        if (lineTokens[i].type == Token::Type::Comma) {
                            continue;
                        }
                        directive.arguments.push_back(lineTokens[i].lexeme);
                    }
                    if (ast.empty()) {
                        LOG_INFO("Implicit .text section created");
                        ast.push_back(Section{ "text", {} });
                    }
                    ast.back().items.push_back(directive);
                }
                else if (auto ignored = magic_enum::enum_cast<IgnoredDirectives>(lineTokens[1].lexeme)) {
                    LOG_WARNING("Ignoring directive '{}' at line {} column {}", lineTokens[1].lexeme, lineTokens[1].line, lineTokens[1].column);
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

            auto possiblePrefixes = populatePossiblePrefixes();
            if (std::ranges::find(possiblePrefixes, lineTokens[0].lexeme) != possiblePrefixes.end()) {
                prefix = lineTokens[0].lexeme;
                mnemonicPos = 1;
            }
            if (instructionDefinitions.contains(lineTokens[mnemonicPos].lexeme)) {
                mnemonicName = lineTokens[mnemonicPos].lexeme;
            }
            else if (instructionDefinitions.contains(lineTokens[mnemonicPos].lexeme.substr(0, lineTokens[mnemonicPos].lexeme.size() - 1))) {
                mnemonicName = lineTokens[mnemonicPos].lexeme.substr(0, lineTokens[mnemonicPos].lexeme.size() - 1);
                suffix = lineTokens[0].lexeme.substr(lineTokens[0].lexeme.size() - 1);
            }

            if (!mnemonicName.empty()) {
                // Instruction found
                Instruction instruction;
                Mnemonic mnemonic;
                mnemonic.mnemonicName = mnemonicName;

                auto& instructionDef = instructionDefinitions[mnemonicName];
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
                    mnemonic.suffix = suffix;
                }
                instruction.mnemonic = mnemonic;
                parseOperands(instruction, lineTokens, ast);
                continue;
            }

            // Labels
            if (lineTokens[0].type == Token::Type::Identifier && lineTokens[1].type == Token::Type::Colon && lineTokens[2].type == Token::Type::EOL) {
                if (ast.empty()) {
                    LOG_INFO("Implicit .text section created");
                    ast.push_back(Section{ "text", {} });
                }
                ast.back().items.push_back(Label{ lineTokens[0].lexeme });
            }
            // Symbol assignments
            else if (lineTokens[0].type == Token::Type::Identifier && lineTokens[1].type == Token::Type::Equal) {
                ast.back().items.push_back(SymbolAssignment{
                    .name = lineTokens[0].lexeme,
                    .expression = Expression{ std::vector<Token>{ lineTokens.begin() + 2, lineTokens.end() - 1 } } // Exclude EOL
                    });
            }
            else {
                LOG_WARNING("Unknown mnemonic '{}' at line {} column {}", lineTokens[0].lexeme, lineTokens[0].line, lineTokens[0].column);
            }
        }
    }

    return 0;

}
