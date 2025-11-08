#pragma once

#include <filesystem>

#include "types.h"

struct Token {
    enum class Type {
        Identifier,
        Dot,
        Comma,
        Semicolon,
        Colon,
        Dash,
        Number,
        String,
        EOL,
        Register,
        Immediate,
        BracketOpen,
        BracketClose,
        Char,
        SymbolType,
    } type;

    std::string lexeme;
    u32 line;
    u32 column;
    u32 length;
};

int lex(std::vector<std::string>& inputLines, std::vector<Token>& tokens);