// SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <filesystem>
#include <iostream>
#include <format>
#include <source_location>

#include "types.h"

enum class LogLevel {
    Error,
    Warning,
    Info,
    Debug,
};

#define RED(value) "\033[31m" << value << "\033[0m"
#define YELLOW(value) "\033[33m" << value << "\033[0m"
#define BLUE(value) "\033[34m" << value << "\033[0m"

inline LogLevel logLevel = LogLevel::Info;

#if MSVC
#define DEBUG_BREAK __debugbreak()
#elif CLANG
#define DEBUG_BREAK __builtin_debugtrap()
#endif

consteval std::string_view shortenPath(const char* inputPath) {
    u16 length = 0;
    while (inputPath[length] != '\0') {
        ++length;
    }
    for (u16 i = 3; i < length; i++) {
        if (inputPath[i - 3] == 's' &&
            inputPath[i - 2] == 'r' &&
            inputPath[i - 1] == 'c' &&
            (inputPath[i] == '/' || inputPath[i] == '\\')) {
            return std::string_view(&inputPath[i + 1], length - i - 1);
        }
    }
    return std::string_view(inputPath, length);
}

#define LOG_ERROR(msg, ...) \
do { \
    if (true) { \
        constexpr auto sourceLocation = std::source_location::current(); \
        std::cout << RED(std::format("{}:{} ({}) ERROR: {}\n", shortenPath(sourceLocation.file_name()), sourceLocation.line(), __func__, std::format(msg __VA_OPT__(,) __VA_ARGS__))) << std::flush; \
    } \
     DEBUG_BREAK; \
     std::exit(1); \
} while (0)

#define LOG_WARNING(msg, ...) \
do { \
    if (logLevel == LogLevel::Warning || logLevel == LogLevel::Info || logLevel == LogLevel::Debug) { \
        constexpr auto sourceLocation = std::source_location::current(); \
        std::cout << YELLOW(std::format("{}:{} ({}) WARNING: {}\n", shortenPath(sourceLocation.file_name()), sourceLocation.line(), __func__, std::format(msg __VA_OPT__(,) __VA_ARGS__))) << std::flush; \
    } \
} while (0)

#define LOG_INFO(msg, ...) \
do { \
    if (logLevel == LogLevel::Info || logLevel == LogLevel::Debug) { \
        constexpr auto sourceLocation = std::source_location::current(); \
        std::cout << std::format("{}:{} ({}) INFO: {}\n", shortenPath(sourceLocation.file_name()), sourceLocation.line(), __func__, std::format(msg __VA_OPT__(,) __VA_ARGS__)) << std::flush; \
    } \
} while (0)

#define LOG_DEBUG(msg, ...) \
do { \
    if (logLevel == LogLevel::Debug) { \
        constexpr auto sourceLocation = std::source_location::current(); \
        std::cout << BLUE(std::format("{}:{} ({}) DEBUG: {}\n", shortenPath(sourceLocation.file_name()), sourceLocation.line(), __func__, std::format(msg __VA_OPT__(,) __VA_ARGS__))) << std::flush; \
    } \
} while (0)
