#pragma once

#include <filesystem>

#include "types.h"

int lex(const std::filesystem::path& inputPath);

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
    } type;

    std::string lexeme;
    u32 line;
    u32 column;
    u32 length;
};