// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cctype>
#include <filesystem>
#include <format>
#include <string>

#include "lexer/lexer.h"
#include "types.h"

int lex(std::vector<std::string>& inputLines, std::vector<Token>& tokens) {
    u32 lineNumber = 0;

    bool inString = false;
    bool inChar = false;
    bool buildingNumber = false;
    bool buildingIdentifier  = false;
    bool buildingImmediate = false;
    bool buildingRegister = false;
    bool buildingSymbolType = false;

    auto endCurrentLexemes = [&](){
        buildingNumber = false;
        inString = false;
        inChar = false;
        buildingIdentifier  = false;
        buildingImmediate = false;
        buildingRegister = false;
        buildingSymbolType = false;
    };

    for (const auto& line : inputLines) {
        ++lineNumber;

        const auto firstNonSpace = line.find_first_not_of(" \t");
        if (firstNonSpace == std::string::npos) {
            continue;
        }

        u32 column = 0;

        endCurrentLexemes();

        bool exitLoop = false;

        for (u32 i = 0; i < line.size(); ++i) {
            char c = line[i];
            if (exitLoop) {
                break;
            }

            auto uc = static_cast<unsigned char>(c);

            if (c == '\"') {
                if (!inString) {
                    tokens.push_back(Token{ Token::Type::String, std::string{}, lineNumber, column, 0 });
                    inString = true;
                } else {
                    inString = false;
                }
                continue;
            }

            if (c == '\'') {
                if (!inChar) {
                    tokens.push_back(Token{ Token::Type::Char, std::string{}, lineNumber, column, 0 });
                    inChar = true;
                } else {
                    inChar = false;
                }
                continue;
            }

            if (inString || inChar) {
                // accumulate raw characters
                tokens.back().lexeme += c;
                tokens.back().length += 1;
                continue;
            }

            switch (c) {
                case ' ':
                    endCurrentLexemes();
                    continue;

                case '\t':
                    endCurrentLexemes();
                    continue;

                case '.':
                    {
                        if (buildingIdentifier) {
                            tokens.back().lexeme += c;
                            tokens.back().length += 1;
                        }
                        else {
                            endCurrentLexemes();
                            tokens.push_back(Token{ Token::Type::Dot, ".", lineNumber, column, 1 });
                        }
                    }
                    continue;

                case ',':
                    endCurrentLexemes();
                    tokens.push_back(Token{ Token::Type::Comma, ",", lineNumber, column, 1 });
                    continue;

                case ';':
                    endCurrentLexemes();
                    tokens.push_back(Token{ Token::Type::Semicolon, ";", lineNumber, column, 1 });
                    continue;

                case '-':
                    {
                        char nextChar = (i + 1 < line.size()) ? line[i + 1] : '\0';
                        unsigned char unsignedNextChar = static_cast<unsigned char>(nextChar);

                        if (std::isdigit(unsignedNextChar) && !buildingIdentifier && !buildingImmediate && !buildingRegister && !buildingSymbolType) {
                            endCurrentLexemes();
                            buildingNumber = true;
                            tokens.push_back(Token{ Token::Type::NegativeNumber, "-", lineNumber, column, 1 });
                        }
                        else {
                            endCurrentLexemes();
                            tokens.push_back(Token{ Token::Type::Dash, "-", lineNumber, column, 1 });
                        }
                    }
                    continue;

                case ':':
                    endCurrentLexemes();
                    tokens.push_back(Token{ Token::Type::Colon, ":", lineNumber, column, 1 });
                    continue;

                case '#':
                    endCurrentLexemes();
                    exitLoop = true;
                    continue;

                case '$':
                    endCurrentLexemes();
                    buildingImmediate = true;
                    tokens.push_back(Token{ Token::Type::Immediate, "$", lineNumber, column, 1 });
                    continue;

                case '%':
                    endCurrentLexemes();
                    buildingRegister = true;
                    tokens.push_back(Token{ Token::Type::Register, "%", lineNumber, column, 1 });
                    continue;

                case '(':
                    endCurrentLexemes();
                    tokens.push_back(Token{ Token::Type::BracketOpen, "(", lineNumber, column, 1 });
                    continue;

                case ')':
                    endCurrentLexemes();
                    tokens.push_back(Token{ Token::Type::BracketClosed, ")", lineNumber, column, 1 });
                    continue;

                case '@':
                    if (buildingIdentifier) {
                        tokens.back().lexeme += c;
                        tokens.back().length += 1;
                    } else if (!buildingImmediate && !buildingRegister && !buildingNumber) {
                        buildingSymbolType = true;
                        tokens.push_back(Token{ Token::Type::SymbolType, "@", lineNumber, column, 1 });
                    }
                    continue;

                case '=':
                    endCurrentLexemes();
                    tokens.push_back(Token{ Token::Type::Equal, "=", lineNumber, column, 1 });
                    continue;

                default:
                    break;
            }

            if (std::isdigit(uc)) {
                if (!buildingNumber && !buildingIdentifier && !buildingImmediate && !buildingRegister) {
                    endCurrentLexemes();
                    buildingNumber = true;
                    tokens.push_back(Token{ Token::Type::Number, std::string(1, c), lineNumber, column, 1 });
                } else {
                    tokens.back().lexeme += c;
                    tokens.back().length += 1;
                }
                continue;
            }

            if (std::isalpha(uc) || c == '_') {
                if (!buildingIdentifier && !buildingRegister && !buildingSymbolType) {
                    endCurrentLexemes();
                    buildingIdentifier = true;
                    tokens.push_back(Token{ Token::Type::Identifier, std::string(1, c), lineNumber, column, 1 });
                } else {
                    tokens.back().lexeme += c;
                    tokens.back().length += 1;
                }
                continue;
            }
            // endCurrentLexemes();
        }
        tokens.push_back(Token{ Token::Type::EOL, std::string{}, lineNumber, column, 0 });
    }
    return 0;
};
