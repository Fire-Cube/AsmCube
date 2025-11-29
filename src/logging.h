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

inline std::string shortenPath(const char* inputPath) {
    std::string normalizedPath = std::filesystem::path(std::string{ inputPath }).generic_string();
    u32 srcPos = normalizedPath.rfind("src/") + 4;
    return normalizedPath.substr(srcPos);
}

#define LOG_ERROR(msg, ...) \
do { \
    if (true) { \
        const auto sourceLocation = std::source_location::current(); \
        std::cout << RED(std::format("{}:{} ({}) ERROR: {}\n", shortenPath(sourceLocation.file_name()), sourceLocation.line(), __func__, std::format(msg, ##__VA_ARGS__))); \
    } \
     __builtin_trap(); \
     std::exit(1); \
} while (0)

#define LOG_WARNING(msg, ...) \
do { \
    if (logLevel == LogLevel::Warning || logLevel == LogLevel::Info || logLevel == LogLevel::Debug) { \
        const auto sourceLocation = std::source_location::current(); \
        std::cout << YELLOW(std::format("{}:{} ({}) WARNING: {}\n", shortenPath(sourceLocation.file_name()), sourceLocation.line(), __func__, std::format(msg, ##__VA_ARGS__))); \
    } \
} while (0)

#define LOG_INFO(msg, ...) \
do { \
    if (logLevel == LogLevel::Info || logLevel == LogLevel::Debug) { \
        const auto sourceLocation = std::source_location::current(); \
        std::cout << std::format("{}:{} ({}) INFO: {}\n", shortenPath(sourceLocation.file_name()), sourceLocation.line(), __func__, std::format(msg, ##__VA_ARGS__)); \
    } \
} while (0)

#define LOG_DEBUG(msg, ...) \
do { \
    if (logLevel == LogLevel::Debug) { \
        const auto sourceLocation = std::source_location::current(); \
        std::cout << BLUE(std::format("{}:{} ({}) DEBUG: {}\n", shortenPath(sourceLocation.file_name()), sourceLocation.line(), __func__, std::format(msg, ##__VA_ARGS__))); \
    } \
} while (0)
