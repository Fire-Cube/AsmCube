#pragma once

#include <string>
#include <vector>
#include <variant>
#include <optional>

#include <cereal/archives/json.hpp>
#include <cereal/types/string.hpp>

#include "cereal_overrides.h"
#include "types.h"
#include "lexer/lexer.h"

struct Label {
    std::string name;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("name", name));
    }
};

struct Directive {
    enum class Name {
        global,
        ascii,
        skip,
    };
    Name name;
    std::vector<std::string> arguments;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("name", name),
                cereal::make_nvp("arguments", arguments));
    }
};

enum class IgnoredDirectives {
    type,
    cfi_startproc,
    cfi_endproc,
    cfi_undefined,
    size,
};

enum class CondCode {
    overflow,
    notOverflow,
    sign,
    notSign,
    equal,
    zero,
    notEqual,
    notZero,
    below,
    notAboveOrEqual,
    carry,
    aboveEqual,
    notCarry,
    belowOrEqual,
    notAbove,
    above,
    notBelowOrEqual,
    less,
    notGreaterOrEqual,
    greaterOrEqual,
    notLess,
    lessOrEqual,
    notGreater,
    greater,
    notLessOrEqual,
    parity,
    parityEven,
    notParity,
    parityOdd,
};

struct Mnemonic {
    std::string mnemonicName;

    std::string prefix;
    std::string suffix;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("mnemonicName", mnemonicName),
                cereal::make_nvp("prefix", prefix),
                cereal::make_nvp("suffix", suffix));
    }
};

struct Register {
    std::string name;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(
            cereal::make_nvp("name", name));
    }
};

struct Immediate {
    std::optional<std::string> symbol;
    std::optional<u64> integer;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("symbol", symbol),
                cereal::make_nvp("integer", integer));
    }
};

enum class Scale {
    One,
    Two,
    Four,
    Eight,
};

struct Memory {
    std::optional<std::variant<u64, Label>> disp;
    std::optional<Register> base;
    std::optional<Register> index;
    std::optional<Scale> scale;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("disp", disp),
                cereal::make_nvp("base", base),
                cereal::make_nvp("index", index),
                cereal::make_nvp("scale", scale));
    }
};

using Operand = std::variant<Register, Immediate, Memory>;

struct Instruction {
    Mnemonic mnemonic;
    std::vector<Operand> operands;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("mnemonic", mnemonic),
                cereal::make_nvp("items", operands));
    }
};

struct Expression {
    std::vector<Token> tokens;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("tokens", tokens));
    }
};

struct SymbolAssignment {
    std::string name;
    Expression expression;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("name", name),
                cereal::make_nvp("expression", expression));
    }
};

struct Section {
    std::string name;
    std::vector<std::variant<Label, Directive, Instruction, SymbolAssignment>> items;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("name", name),
                cereal::make_nvp("items", items));
        }
};

using Ast = std::vector<Section>;

int parseOperand(Instruction& instruction, const std::vector<Token>& lineTokens, const u32 operandStart, const std::vector<u32>& operandCommaPositions);
int parse(const std::vector<Token>& tokens, std::vector<Section>& ast);