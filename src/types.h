// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

constexpr u64 operator""_KiB(u64 value) {
    return value * 1024;
}

constexpr u64 operator""_MiB(u64 value) {
    return value * 1024 * 1024;
}

constexpr u64 operator""_GiB(u64 value) {
    return value * 1024 * 1024 * 1024;
}

constexpr u64 operator""_KB(u64 value) {
    return value * 1000;
}

constexpr u64 operator""_MB(u64 value) {
    return value * 1000 * 1000;
}

constexpr u64 operator""_GB(u64 value) {
    return value * 1000 * 1000 * 1000;
}
