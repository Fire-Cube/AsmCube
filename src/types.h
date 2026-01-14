// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>

#if defined(__clang__)
    #define CLANG 1
#else
    #define CLANG 0
#endif

#if !defined(__clang__) && defined(_MSC_VER)
    #define MSVC 1
#else
    #define MSVC 0
#endif

#if MSVC
#include <__msvc_int128.hpp>
#endif

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;
#if MSVC
using s128 = std::_Signed128;
#elif CLANG
using s128 = __int128_t;
#endif


using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
#if MSVC
using u128 = std::_Unsigned128;
#elif CLANG
using u128 = __uint128_t;
#endif

constexpr u64 operator""_KiB(unsigned long long value) {
    return value * 1024;
}

constexpr u64 operator""_MiB(unsigned long long value) {
    return value * 1024 * 1024;
}

constexpr u64 operator""_GiB(unsigned long long value) {
    return value * 1024 * 1024 * 1024;
}

constexpr u64 operator""_KB(unsigned long long value) {
    return value * 1000;
}

constexpr u64 operator""_MB(unsigned long long value) {
    return value * 1000 * 1000;
}

constexpr u64 operator""_GB(unsigned long long value) {
    return value * 1000 * 1000 * 1000;
}
