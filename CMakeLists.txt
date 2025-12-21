# SPDX-FileCopyrightText: Copyright 2025 AsmCube Project
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.24)

project(AsmCube LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE True)

set(SOURCES
    src/cereal_overrides.h
    src/magic_enum_overrides.h
    src/lexer/lexer.cpp
    src/lexer/lexer.h
    src/parser/parser.cpp
    src/parser/parser.h
    src/interpreter/instructions.cpp
    src/interpreter/instructions.h
    src/interpreter/interpreter.cpp
    src/interpreter/interpreter.h
    src/interpreter/memory.h
    src/interpreter/registers.h
    src/interpreter/syscalls.cpp
    src/interpreter/syscalls.h
    src/main.cpp
)

SET(EXTERNALS
    src/externals/cereal/include
    src/externals/argparse/include
    src/externals/magic_enum/include
)

add_executable(AsmCube ${SOURCES})

target_include_directories(AsmCube PRIVATE
    ${EXTERNALS}
    src
)