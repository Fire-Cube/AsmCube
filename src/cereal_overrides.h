#pragma once

#include <string>
#include <optional>
#include <variant>
#include <source_location>

#include <cereal/cereal.hpp>
#include <magic_enum/magic_enum.hpp>

#include "types.h"

namespace cereal {

template <class Archive, typename Enum>
requires std::is_enum_v<Enum>
std::string save_minimal(Archive& archive, const Enum& e) {
    return std::string{ magic_enum::enum_name(e) };
}

template <class Archive, typename T>
void save(Archive& archive, const std::optional<T>& optional) {
    if (optional) {
        archive(cereal::make_nvp("data", *optional));
    }
}

template <typename T>
std::string getTypeName() {
    auto sourceInfo = std::source_location::current();

    std::string functionString = sourceInfo.function_name();
    u32 typeNameStart = functionString.rfind("T = ") + 4;
    u32 typeNameEnd = functionString.rfind(']');

    return functionString.substr(typeNameStart, typeNameEnd - typeNameStart);
}

template <class Archive, class... Ts>
inline void save(Archive& archive, const std::variant<Ts...>& variant) {
    std::visit([&](auto const& alt) {
        using T = std::decay_t<decltype(alt)>;
        archive(cereal::make_nvp("type", getTypeName<T>()),
                cereal::make_nvp("data", alt));
    }, variant);
}

} // namespace cereal