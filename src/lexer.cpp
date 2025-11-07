#include <cctype>
#include <filesystem>
#include <fstream>
#include <format>
#include <iostream>
#include <string>

#include "lexer.h"

#include "types.h"

int lex(const std::filesystem::path& inputPath) {
    if (!std::filesystem::exists(inputPath)) {
        std::cout << std::format("File '{}' does not exist!\n", inputPath.string());
        return 1;
    }

    std::ifstream in(inputPath, std::ios::binary);
    if (!in) {
        std::cout << std::format("Failed to open file '{}'\n", inputPath.string());
        return 1;
    }

    std::vector<Token> tokens;
    std::string line;
    u32 lineNumber = 0;

    bool inString = false;
    bool inChar = false;
    bool buildingNumber = false;
    bool buildingIdentifier  = false;
    bool buildingImmediate = false;
    bool buildingRegister = false;

    auto endCurrentLexemes = [&](){
        buildingNumber = false;
        inChar = false;
        buildingIdentifier  = false;
        buildingImmediate = false;
        buildingRegister = false;
    };

    while (std::getline(in, line)) {
        ++lineNumber;

        // Skip blank/space-only lines
        const auto firstNonSpace = line.find_first_not_of(' ');
        if (firstNonSpace == std::string::npos) {
            continue;
        }

        u32 column = 0;

        inString = false;
        inChar = false;
        buildingNumber = false;
        buildingIdentifier  = false;
        buildingImmediate = false;
        buildingRegister = false;

        bool exitLoop = false;

        for (char c : line) {
            if (exitLoop) {
                break;
            }

            auto uc = static_cast<unsigned char>(c);
            ++column;

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

                case '.':
                    endCurrentLexemes();
                    tokens.push_back(Token{ Token::Type::Dot, ".", lineNumber, column, 1 });
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
                    endCurrentLexemes();
                    tokens.push_back(Token{ Token::Type::Dash, "-", lineNumber, column, 1 });
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
                    tokens.push_back(Token{ Token::Type::BracketClose, ")", lineNumber, column, 1 });
                    continue;

                default:
                    break;
            }

            if (std::isdigit(uc)) {
                if (!buildingNumber & !buildingIdentifier & !buildingImmediate & !buildingRegister) {
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
                if (!buildingIdentifier & !buildingRegister) {
                    endCurrentLexemes();
                    buildingIdentifier = true;
                    tokens.push_back(Token{ Token::Type::Identifier, std::string(1, c), lineNumber, column, 1 });
                } else {
                    tokens.back().lexeme += c;
                    tokens.back().length += 1;
                }
                continue;
            }
            endCurrentLexemes();
        }
        tokens.push_back(Token{ Token::Type::EOL, std::string{}, lineNumber, column, 0 });
    }
    return 0;
};
