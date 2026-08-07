// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <optional>
#include <variant>

#include <cereal/archives/json.hpp>

#include "types.h"

namespace Ast
{

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
    notBelow,
    aboveOrEqual,
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
        globl,
        ascii,
        asciz,
        quad,
        byte,
        skip,
        space,
        zero,
    };
    Name name;
    std::vector<std::string> arguments;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("name", name),
                cereal::make_nvp("arguments", arguments));
    }
};

enum class Width : u8 {
    Byte = 8,
    Word = 16,
    Long = 32,
    Quad = 64,
};

struct Mnemonic {
    std::string mnemonicName;

    std::string prefix;
    std::optional<Width> width;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("mnemonicName", mnemonicName),
                cereal::make_nvp("prefix", prefix),
                cereal::make_nvp("width", width));
    }
};



struct Register {
    std::string name;
    Width width;
    u8 index;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(
            cereal::make_nvp("name", name),
            cereal::make_nvp("width", width),
            cereal::make_nvp("index", index));
    }
};

struct Symbol {
    std::string name;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("name", name));
    }
};

struct Immediate {
    u64 value;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("value", value));
    }
};

struct RelativeImmediate {
    std::variant<s64, Label> target;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("target", target));
    }
};

enum class Scale {
    One,
    Two,
    Four,
    Eight,
};

struct Memory {
    std::optional<std::variant<s64, Label>> disp;
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

using Operand = std::variant<Register, RelativeImmediate, Immediate, Memory, Symbol>;

enum class OperandType {
    Register,
    RelativeImmediate,
    Immediate,
    Memory,
    Symbol,
};

struct Instruction {
    Mnemonic mnemonic;
    std::vector<Operand> operands;
    std::optional<std::variant<CondCode>> additionalData;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("mnemonic", mnemonic),
                cereal::make_nvp("items", operands),
                cereal::make_nvp("additionalData", additionalData));
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

} // namespace Ast