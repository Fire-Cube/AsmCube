#pragma once

#include <string>

#include <cereal/archives/json.hpp>
#include <cereal/types/string.hpp>

#include "cereal_overrides.h"
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
        BracketClosed,
        Char,
        SymbolType,
        Equal,
    } type;

    std::string lexeme;
    u32 line;
    u32 column;
    u32 length;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("type", type),
                cereal::make_nvp("lexeme", lexeme),
                cereal::make_nvp("line", line),
                cereal::make_nvp("column", column),
                cereal::make_nvp("length", length));
    }
};

int lex(std::vector<std::string>& inputLines, std::vector<Token>& tokens);