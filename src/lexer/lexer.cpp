// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cctype>
#include <filesystem>
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
    bool buildingHexNumber = false;

    auto endCurrentLexemes = [&](){
        buildingNumber = false;
        inString = false;
        inChar = false;
        buildingIdentifier  = false;
        buildingImmediate = false;
        buildingRegister = false;
        buildingSymbolType = false;
        buildingHexNumber = false;
    };

    for (const auto& line : inputLines) {
        bool exitLoop = false;
        bool firstDotWasSplitted = false;

        ++lineNumber;

        endCurrentLexemes();

        const auto firstNonSpace = line.find_first_not_of(" \t");
        if (firstNonSpace == std::string::npos) {
            continue;
        }

        for (u32 i = 0; i < line.size(); ++i) {
            u32& column = i;
            const char c = line[i];
            if (exitLoop) {
                break;
            }

            const auto uc = static_cast<unsigned char>(c);

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
                            if (column == firstNonSpace && !firstDotWasSplitted) {
                                firstDotWasSplitted = true;
                                endCurrentLexemes();
                                tokens.push_back(Token{ Token::Type::Dot, ".", lineNumber, column, 1 });
                            }
                            else if (firstDotWasSplitted && (std::isdigit(line[column + 1]) || std::isalpha(line[column + 1]) || line[column] == '_') || tokens.back().type == Token::Type::Identifier) {
                                buildingIdentifier = true;
                                tokens.push_back(Token{ Token::Type::Identifier, std::string(1, c), lineNumber, column, 1 });
                            }
                            else {
                                endCurrentLexemes();
                                tokens.push_back(Token{ Token::Type::Dot, ".", lineNumber, column, 1 });
                            }
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

                case '0':
                    if (line[i + 1] == 'x') {
                        buildingHexNumber = true;
                        if (!buildingImmediate) {
                            tokens.push_back(Token{ Token::Type::HexNumber, "0", lineNumber, column, 1 });
                            continue;
                        }
                    }

                case 'x':
                    if (buildingHexNumber && tokens.back().length == 1) {
                        tokens.back().lexeme += c;
                        tokens.back().length += 1;
                        continue;
                    }

                default:
                    break;
            }

            if (std::isdigit(uc)) {
                if (!buildingNumber && !buildingIdentifier && !buildingImmediate && !buildingRegister && !buildingHexNumber) {
                    endCurrentLexemes();
                    buildingNumber = true;
                    tokens.push_back(Token{ Token::Type::Number, std::string(1, c), lineNumber, column, 1 });
                } else {
                    tokens.back().lexeme += c;
                    tokens.back().length += 1;
                }
                continue;
            }

            if (buildingHexNumber && std::isalpha(uc) && (std::tolower(uc) >= 'a' && std::tolower(uc) <= 'f')) {
                tokens.back().lexeme += c;
                tokens.back().length += 1;
                continue;
            }

            if (std::isalpha(uc) || c == '_') {
                if (!buildingIdentifier && !buildingRegister && !buildingSymbolType && !buildingHexNumber) {
                    endCurrentLexemes();
                    buildingIdentifier = true;
                    tokens.push_back(Token{ Token::Type::Identifier, std::string(1, c), lineNumber, column, 1 });
                } else {
                    tokens.back().lexeme += c;
                    tokens.back().length += 1;
                }
            }
        }
        tokens.push_back(Token{ Token::Type::EOL, std::string{}, lineNumber, static_cast<u32>(inputLines[lineNumber - 1].length()), 0 });
    }
    return 0;
};
