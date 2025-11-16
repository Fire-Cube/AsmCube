#pragma once

#include <magic_enum/magic_enum.hpp>

#include "parser.h"

namespace magic_enum::customize {

template<>
constexpr customize_t enum_name<NormalMnemonic>(NormalMnemonic value) noexcept {
    switch (value) {
    case NormalMnemonic::Xor:
            return "xor";

        default:
            return default_tag;
    }
}

} // namespace magic_enum::customize
